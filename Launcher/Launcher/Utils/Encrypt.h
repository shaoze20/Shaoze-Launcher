#pragma once
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")
#endif

namespace LocalDataStore {

    static inline QString dataFilePath()
    {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        return dir + "/data.dat";
    }

#ifdef Q_OS_WIN
    static inline QByteArray dpapiEncrypt(const QByteArray& plain)
    {
        if (plain.isEmpty()) return {};
        DATA_BLOB in{ (DWORD)plain.size(), (BYTE*)plain.data() };
        DATA_BLOB out{ 0, nullptr };

        if (!CryptProtectData(&in, L"LocalData", nullptr, nullptr, nullptr, 0, &out))
            return {};

        QByteArray enc((char*)out.pbData, (int)out.cbData);
        LocalFree(out.pbData);
        return enc;
    }

    static inline QByteArray dpapiDecrypt(const QByteArray& enc)
    {
        if (enc.isEmpty()) return {};
        DATA_BLOB in{ (DWORD)enc.size(), (BYTE*)enc.data() };
        DATA_BLOB out{ 0, nullptr };

        if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out))
            return {};

        QByteArray plain((char*)out.pbData, (int)out.cbData);
        LocalFree(out.pbData);
        return plain;
    }
#endif

    // 读取结果结构体
    struct Data
    {
        QString username;   // 默认空
        QString password;   // 默认空
        bool autoLogin = false;
        bool ok = false;    // data.dat 是否存在且 JSON 合法
    };

    // 内部
    static inline QJsonObject loadRootObject(bool* ok = nullptr)
    {
        if (ok) *ok = false;

        QFile f(dataFilePath());
        if (!f.exists()) return QJsonObject{};
        if (!f.open(QIODevice::ReadOnly)) return QJsonObject{};

        QJsonParseError err{};
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) return QJsonObject{};

        if (ok) *ok = true;
        return doc.object();
    }

    // 保存
    static inline bool saveAll(const QString& username,const QString& password,bool autoLogin)
    {
        QJsonObject root;
        root["username"] = username;
        root["autoLogin"] = autoLogin;

#ifdef Q_OS_WIN
        const QByteArray enc = dpapiEncrypt(password.toUtf8());
        root["pwd_b64"] = QString::fromLatin1(enc.toBase64());
#else
        root["pwd_b64"] = QString();
#endif

        const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);

        QSaveFile f(dataFilePath());
        if (!f.open(QIODevice::WriteOnly)) return false;
        if (f.write(json) != json.size()) return false;
        return f.commit();
    }

    // ------------------------------
    // 保存（部分更新）：满足你“参数可空，不想覆盖旧值”的需求
    //
    // 规则：
    // - 传 QString()（isNull()==true） => 不更新该字段
    // - 传 QString("")（空字符串但非 null） => 更新为空（清空）
    // - autoLogin: -1 不更新；0/1 更新
    // ------------------------------
    static inline bool savePartial(const QString& username,const QString& password,int autoLogin)
    {
        bool ok = false;
        QJsonObject root = loadRootObject(&ok);
        if (!ok) root = QJsonObject{};

        if (!username.isNull())  root["username"] = username;
        if (autoLogin != -1)     root["autoLogin"] = (autoLogin != 0);

        if (!password.isNull()) {
#ifdef Q_OS_WIN
            const QByteArray enc = dpapiEncrypt(password.toUtf8());
            root["pwd_b64"] = QString::fromLatin1(enc.toBase64());
#else
            root["pwd_b64"] = QString();
#endif
        }

        const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);

        QSaveFile f(dataFilePath());
        if (!f.open(QIODevice::WriteOnly)) return false;
        if (f.write(json) != json.size()) return false;
        return f.commit();
    }

    // 读取
    static inline Data load()
    {
        Data d; // 默认都为空/false
        bool ok = false;
        const QJsonObject root = loadRootObject(&ok);
        d.ok = ok;
        if (!ok) return d;

        d.username = root.value("username").toString();
        d.autoLogin = root.value("autoLogin").toBool(false);

#ifdef Q_OS_WIN
        const QByteArray b64 = root.value("pwd_b64").toString().toLatin1();
        const QByteArray enc = QByteArray::fromBase64(b64);
        d.password = QString::fromUtf8(dpapiDecrypt(enc));
#else
        d.password.clear();
#endif

        return d;
    }

    // ------------------------------
    // 清除：删除 data.dat
    // ------------------------------
    static inline bool clear()
    {
        QFile f(dataFilePath());
        if (!f.exists()) return true;
        return f.remove();
    }

} // namespace LocalDataStore
