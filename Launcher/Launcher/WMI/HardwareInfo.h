#pragma once

#ifndef HARDWAREINFO_H
#define HARDWAREINFO_H

#include <cmath>
#include <QSysInfo>
#include <QString>
#include <QSet>
#include <QStringList>
#include <QList>
#include <qDebug>
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <dxgi.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "wbemuuid.lib")

struct MemoryModuleInfo
{
    QString manufacturer;
    QString partNumber;
    QString capacity;
    QString speed;
};

class HardwareInfo{
public:
    static QString systemInfo();
    static QString uptime();
    static QString cpuName();
    static QString motherboard();
    static QString totalMemory();
    static QList<MemoryModuleInfo> memoryModules();
    static QStringList monitors();
    static QStringList diskDrives();
    static QStringList soundCards();
    static QStringList networkCards();
    static QStringList graphicsCards();

private:
    struct WmiContext
    {
        IWbemLocator* loc = nullptr;
        IWbemServices* svc = nullptr;
        bool needUninit = false;
    };

    static bool initializeWMI(WmiContext& ctx);
    static void cleanupWMI(WmiContext& ctx);

    static QString querySingleString(const QString& wmiClass,
        const QString& property,
        const QString& nameSpace = "ROOT\\CIMV2");

    static QStringList queryStringList(const QString& wmiClass,
        const QString& property,
        const QString& condition = QString(),
        const QString& nameSpace = "ROOT\\CIMV2");

    static QString variantToQString(const tagVARIANT& vt);
    static QString formatBytes(qulonglong bytes);
};

#endif // HARDWAREINFO_H