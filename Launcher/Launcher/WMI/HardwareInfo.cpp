#include "HardwareInfo.h"

namespace
{
    QString decodeMonitorUShortArray(VARIANT& vt)
    {
        if ((vt.vt & VT_ARRAY) == 0 || vt.parray == nullptr)
            return QString();

        SAFEARRAY* psa = vt.parray;
        LONG lBound = 0;
        LONG uBound = -1;
        SafeArrayGetLBound(psa, 1, &lBound);
        SafeArrayGetUBound(psa, 1, &uBound);

        QString result;
        for (LONG i = lBound; i <= uBound; ++i) {
            USHORT value = 0;
            if (SUCCEEDED(SafeArrayGetElement(psa, &i, &value))) {
                if (value == 0)
                    break;
                result.append(QChar(static_cast<ushort>(value)));
            }
        }
        return result.trimmed();
    }

    QMap<QString, QString> queryGpuMemoryByDxgi()
    {
        QMap<QString, QString> result;

        IDXGIFactory* factory = nullptr;
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
        if (FAILED(hr) || !factory)
            return result;

        UINT index = 0;
        IDXGIAdapter* adapter = nullptr;

        while (factory->EnumAdapters(index, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC desc;
            ZeroMemory(&desc, sizeof(desc));

            if (SUCCEEDED(adapter->GetDesc(&desc))) {
                QString name = QString::fromWCharArray(desc.Description).trimmed();

                if (!name.isEmpty() && desc.DedicatedVideoMemory > 0) {
                    const double gb = static_cast<double>(desc.DedicatedVideoMemory)
                        / (1024.0 * 1024.0 * 1024.0);
                    result[name] = QString::number(gb, 'f', 1) + " GB";
                }
            }

            adapter->Release();
            adapter = nullptr;
            ++index;
        }

        factory->Release();
        return result;
    }

    double calcInch(int widthCm, int heightCm)
    {
        if (widthCm <= 0 || heightCm <= 0)
            return 0.0;

        double diagonalCm = std::sqrt(widthCm * widthCm + heightCm * heightCm);
        return diagonalCm / 2.54;
    }
}

bool HardwareInfo::initializeWMI(WmiContext& ctx)
{
    ctx.loc = nullptr;
    ctx.svc = nullptr;
    ctx.needUninit = false;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        ctx.needUninit = true;
    }
    else if (hr != RPC_E_CHANGED_MODE) {
        return false;
    }

    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr
    );

    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        cleanupWMI(ctx);
        return false;
    }

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&ctx.loc)
    );

    if (FAILED(hr) || !ctx.loc) {
        cleanupWMI(ctx);
        return false;
    }

    hr = ctx.loc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        nullptr,
        nullptr,
        0,
        NULL,
        0,
        0,
        &ctx.svc
    );

    if (FAILED(hr) || !ctx.svc) {
        cleanupWMI(ctx);
        return false;
    }

    hr = CoSetProxyBlanket(
        ctx.svc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE
    );

    if (FAILED(hr)) {
        cleanupWMI(ctx);
        return false;
    }

    return true;
}

void HardwareInfo::cleanupWMI(WmiContext& ctx)
{
    if (ctx.svc) {
        ctx.svc->Release();
        ctx.svc = nullptr;
    }

    if (ctx.loc) {
        ctx.loc->Release();
        ctx.loc = nullptr;
    }

    if (ctx.needUninit) {
        CoUninitialize();
        ctx.needUninit = false;
    }
}

QString HardwareInfo::variantToQString(const tagVARIANT& vt)
{
    switch (vt.vt) {
    case VT_BSTR:
        return vt.bstrVal ? QString::fromWCharArray(vt.bstrVal).trimmed() : QString();
    case VT_I1:
        return QString::number(vt.cVal);
    case VT_UI1:
        return QString::number(vt.bVal);
    case VT_I2:
        return QString::number(vt.iVal);
    case VT_UI2:
        return QString::number(vt.uiVal);
    case VT_I4:
    case VT_INT:
        return QString::number(vt.intVal);
    case VT_UI4:
    case VT_UINT:
        return QString::number(vt.uintVal);
    case VT_I8:
        return QString::number(vt.llVal);
    case VT_UI8:
        return QString::number(vt.ullVal);
    case VT_BOOL:
        return vt.boolVal ? "true" : "false";
    case VT_NULL:
    case VT_EMPTY:
        return QString();
    default:
        break;
    }

    VARIANT dest;
    VariantInit(&dest);
    if (SUCCEEDED(VariantChangeType(&dest, const_cast<VARIANT*>(&vt), 0, VT_BSTR))) {
        QString str = dest.bstrVal ? QString::fromWCharArray(dest.bstrVal).trimmed() : QString();
        VariantClear(&dest);
        return str;
    }

    return QString();
}

QString HardwareInfo::querySingleString(const QString& wmiClass,
    const QString& property,
    const QString& nameSpace)
{
    WmiContext ctx;
    if (!initializeWMI(ctx))
        return QString();

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;
    Q_UNUSED(pLoc);

    if (nameSpace.compare("ROOT\\CIMV2", Qt::CaseInsensitive) != 0) {
        if (ctx.svc) {
            ctx.svc->Release();
            ctx.svc = nullptr;
        }
        pSvc = nullptr;

        HRESULT hr = ctx.loc->ConnectServer(
            _bstr_t(reinterpret_cast<const wchar_t*>(nameSpace.utf16())),
            nullptr,
            nullptr,
            0,
            NULL,
            0,
            0,
            &pSvc
        );

        if (FAILED(hr) || !pSvc) {
            cleanupWMI(ctx);
            return QString();
        }

        ctx.svc = pSvc;

        hr = CoSetProxyBlanket(
            pSvc,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE
        );

        if (FAILED(hr)) {
            cleanupWMI(ctx);
            return QString();
        }
    }

    const QString query = QString("SELECT %1 FROM %2").arg(property, wmiClass);

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return QString();
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;
    QString result;

    hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
    if (SUCCEEDED(hr) && returned > 0 && pObj) {
        VARIANT vtProp;
        VariantInit(&vtProp);

        if (SUCCEEDED(pObj->Get(reinterpret_cast<const wchar_t*>(property.utf16()), 0, &vtProp, nullptr, nullptr))) {
            result = variantToQString(vtProp);
        }

        VariantClear(&vtProp);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);
    return result;
}

QStringList HardwareInfo::queryStringList(const QString& wmiClass,
    const QString& property,
    const QString& condition,
    const QString& nameSpace)
{
    QStringList result;

    WmiContext ctx;
    if (!initializeWMI(ctx))
        return QStringList();

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;
    Q_UNUSED(pLoc);

    if (nameSpace.compare("ROOT\\CIMV2", Qt::CaseInsensitive) != 0) {
        if (ctx.svc) {
            ctx.svc->Release();
            ctx.svc = nullptr;
        }
        pSvc = nullptr;

        HRESULT hr = ctx.loc->ConnectServer(
            _bstr_t(reinterpret_cast<const wchar_t*>(nameSpace.utf16())),
            nullptr,
            nullptr,
            0,
            NULL,
            0,
            0,
            &pSvc
        );

        if (FAILED(hr) || !pSvc) {
            cleanupWMI(ctx);
            return result;
        }

        ctx.svc = pSvc;

        hr = CoSetProxyBlanket(
            pSvc,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE
        );

        if (FAILED(hr)) {
            cleanupWMI(ctx);
            return result;
        }
    }

    QString query = QString("SELECT %1 FROM %2").arg(property, wmiClass);
    if (!condition.trimmed().isEmpty())
        query += " WHERE " + condition;

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return result;
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
        if (FAILED(hr) || returned == 0)
            break;

        VARIANT vtProp;
        VariantInit(&vtProp);

        if (SUCCEEDED(pObj->Get(reinterpret_cast<const wchar_t*>(property.utf16()), 0, &vtProp, nullptr, nullptr))) {
            const QString text = variantToQString(vtProp);
            if (!text.isEmpty() && !result.contains(text))
                result << text;
        }

        VariantClear(&vtProp);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);
    return result;
}

QString HardwareInfo::formatBytes(qulonglong bytes)
{
    const double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
    return QString::number(gb, 'f', 2) + " GB";
}

QString HardwareInfo::systemInfo()
{
    QString osName = QSysInfo::prettyProductName();
    QString cpuArch = QSysInfo::currentCpuArchitecture();
    QString kernelType = QSysInfo::kernelType();
    QString kernelVersion = QSysInfo::kernelVersion();

    QString text;
    text += QString("操作系统：%1").arg(osName.isEmpty() ? "Unknown" : osName);
    text += QString("\n架构：%1").arg(cpuArch.isEmpty() ? "Unknown" : cpuArch);
    text += QString("\n内核类型：%1").arg(kernelType.isEmpty() ? "Unknown" : kernelType);
    text += QString("\n内核版本：%1").arg(kernelVersion.isEmpty() ? "Unknown" : kernelVersion);

    return text;
}

QString HardwareInfo::uptime()
{
    ULONGLONG ms = GetTickCount64();
    quint64 totalSeconds = ms / 1000;

    quint64 days = totalSeconds / (24 * 3600);
    totalSeconds %= (24 * 3600);

    quint64 hours = totalSeconds / 3600;
    totalSeconds %= 3600;

    quint64 minutes = totalSeconds / 60;
    quint64 seconds = totalSeconds % 60;

    QStringList parts;
    if (days > 0)
        parts << QString("%1 天").arg(days);
    if (hours > 0)
        parts << QString("%1 小时").arg(hours);
    if (minutes > 0)
        parts << QString("%1 分钟").arg(minutes);

    parts << QString("%1 秒").arg(seconds);

    return parts.join(" ");
}

QString HardwareInfo::cpuName()
{
    WmiContext ctx;
    if (!initializeWMI(ctx))
        return QString();

    IWbemServices* pSvc = ctx.svc;

    const QString query = "SELECT Name, NumberOfCores, NumberOfLogicalProcessors FROM Win32_Processor";

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return QString();
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    QString name;
    int cores = 0;
    int threads = 0;

    hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
    if (SUCCEEDED(hr) && returned > 0 && pObj) {
        VARIANT vtName, vtCores, vtThreads;
        VariantInit(&vtName);
        VariantInit(&vtCores);
        VariantInit(&vtThreads);

        // Name
        if (SUCCEEDED(pObj->Get(L"Name", 0, &vtName, nullptr, nullptr)))
            name = variantToQString(vtName);

        // Cores
        if (SUCCEEDED(pObj->Get(L"NumberOfCores", 0, &vtCores, nullptr, nullptr)))
            cores = variantToQString(vtCores).toInt();

        // Threads
        if (SUCCEEDED(pObj->Get(L"NumberOfLogicalProcessors", 0, &vtThreads, nullptr, nullptr)))
            threads = variantToQString(vtThreads).toInt();

        VariantClear(&vtName);
        VariantClear(&vtCores);
        VariantClear(&vtThreads);

        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);

    if (name.isEmpty())
        return QString();

    QString suffix;

    if (cores > 0 && threads > 0)
        suffix = QString("%1 核 %2 线程").arg(cores).arg(threads);
    else if (cores > 0)
        suffix = QString("%1 核").arg(cores);

    if (!suffix.isEmpty())
        return QString("%1（%2）").arg(name.trimmed()).arg(suffix);

    return name.trimmed();
}

QString HardwareInfo::motherboard()
{
    WmiContext ctx;
    if (!initializeWMI(ctx))
        return QString();

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;

    const QString query = "SELECT Manufacturer, Product FROM Win32_BaseBoard";

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return QString();
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;
    QString manufacturer;
    QString product;

    hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
    if (SUCCEEDED(hr) && returned > 0 && pObj) {
        VARIANT vtManufacturer, vtProduct;
        VariantInit(&vtManufacturer);
        VariantInit(&vtProduct);

        pObj->Get(L"Manufacturer", 0, &vtManufacturer, nullptr, nullptr);
        pObj->Get(L"Product", 0, &vtProduct, nullptr, nullptr);

        manufacturer = variantToQString(vtManufacturer);
        product = variantToQString(vtProduct);

        VariantClear(&vtManufacturer);
        VariantClear(&vtProduct);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);

    if (!manufacturer.isEmpty() && !product.isEmpty())
        return manufacturer + " " + product;
    return manufacturer.isEmpty() ? product : manufacturer;
}

QString HardwareInfo::totalMemory()
{
    const QString text = querySingleString("Win32_ComputerSystem", "TotalPhysicalMemory");
    bool ok = false;
    qulonglong bytes = text.toULongLong(&ok);
    return ok ? formatBytes(bytes) : text;
}

QList<MemoryModuleInfo> HardwareInfo::memoryModules()
{
    QList<MemoryModuleInfo> list;

    WmiContext ctx;
    if (!initializeWMI(ctx))
        return {};

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;

    const QString query = "SELECT Manufacturer, PartNumber, Capacity, Speed FROM Win32_PhysicalMemory";

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return list;
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
        if (FAILED(hr) || returned == 0)
            break;

        MemoryModuleInfo info;

        VARIANT vtManufacturer, vtPartNumber, vtCapacity, vtSpeed;
        VariantInit(&vtManufacturer);
        VariantInit(&vtPartNumber);
        VariantInit(&vtCapacity);
        VariantInit(&vtSpeed);

        pObj->Get(L"Manufacturer", 0, &vtManufacturer, nullptr, nullptr);
        pObj->Get(L"PartNumber", 0, &vtPartNumber, nullptr, nullptr);
        pObj->Get(L"Capacity", 0, &vtCapacity, nullptr, nullptr);
        pObj->Get(L"Speed", 0, &vtSpeed, nullptr, nullptr);

        info.manufacturer = variantToQString(vtManufacturer);
        info.partNumber = variantToQString(vtPartNumber).trimmed();

        bool ok = false;
        qulonglong bytes = variantToQString(vtCapacity).toULongLong(&ok);
        info.capacity = ok ? formatBytes(bytes) : variantToQString(vtCapacity);

        const QString speed = variantToQString(vtSpeed);
        info.speed = speed.isEmpty() ? QString() : (speed + " MHz");

        VariantClear(&vtManufacturer);
        VariantClear(&vtPartNumber);
        VariantClear(&vtCapacity);
        VariantClear(&vtSpeed);

        list.append(info);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);
    return list;
}

QStringList HardwareInfo::monitors()
{
    QStringList result;

    WmiContext ctx;
    if (!initializeWMI(ctx))
        return {};

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;

    if (ctx.svc) {
        ctx.svc->Release();
        ctx.svc = nullptr;
    }
    pSvc = nullptr;

    HRESULT hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\WMI"),
        nullptr,
        nullptr,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hr) || !pSvc) {
        cleanupWMI(ctx);
        return result;
    }

    ctx.svc = pSvc;

    hr = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE
    );

    if (FAILED(hr)) {
        cleanupWMI(ctx);
        return result;
    }

    const QString query = "SELECT InstanceName, ManufacturerName, UserFriendlyName FROM WmiMonitorID";

    QMap<QString, double> monitorSizeMap;

    {
        IEnumWbemClassObject* pEnumSize = nullptr;
        HRESULT hr2 = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t(L"SELECT InstanceName, MaxHorizontalImageSize, MaxVerticalImageSize FROM WmiMonitorBasicDisplayParams"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &pEnumSize
        );

        if (SUCCEEDED(hr2) && pEnumSize) {
            IWbemClassObject* pObj2 = nullptr;
            ULONG ret = 0;

            while (pEnumSize->Next(WBEM_INFINITE, 1, &pObj2, &ret) == S_OK && ret > 0) {
                VARIANT vtName, vtW, vtH;
                VariantInit(&vtName);
                VariantInit(&vtW);
                VariantInit(&vtH);

                QString instance;
                int w = 0, h = 0;

                if (SUCCEEDED(pObj2->Get(L"InstanceName", 0, &vtName, nullptr, nullptr)))
                    instance = variantToQString(vtName);

                if (SUCCEEDED(pObj2->Get(L"MaxHorizontalImageSize", 0, &vtW, nullptr, nullptr)))
                    w = variantToQString(vtW).toInt();

                if (SUCCEEDED(pObj2->Get(L"MaxVerticalImageSize", 0, &vtH, nullptr, nullptr)))
                    h = variantToQString(vtH).toInt();

                double inch = calcInch(w, h);
                if (!instance.isEmpty() && inch > 0)
                    monitorSizeMap[instance] = inch;

                VariantClear(&vtName);
                VariantClear(&vtW);
                VariantClear(&vtH);
                pObj2->Release();
            }

            pEnumSize->Release();
        }
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return result;
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
        if (FAILED(hr) || returned == 0)
            break;

        VARIANT vtInstanceName, vtManufacturerName, vtUserFriendlyName;
        VariantInit(&vtInstanceName);
        VariantInit(&vtManufacturerName);
        VariantInit(&vtUserFriendlyName);

        pObj->Get(L"InstanceName", 0, &vtInstanceName, nullptr, nullptr);
        pObj->Get(L"ManufacturerName", 0, &vtManufacturerName, nullptr, nullptr);
        pObj->Get(L"UserFriendlyName", 0, &vtUserFriendlyName, nullptr, nullptr);

        QString instanceName = variantToQString(vtInstanceName);
        QString manufacturer = decodeMonitorUShortArray(vtManufacturerName);
        QString name = decodeMonitorUShortArray(vtUserFriendlyName);

        QString text;
        if (!manufacturer.isEmpty() && !name.isEmpty())
            text = manufacturer + " " + name;
        else if (!name.isEmpty())
            text = name;
        else
            text = manufacturer;

        double inch = 0.0;
        if (monitorSizeMap.contains(instanceName)) {
            inch = monitorSizeMap.value(instanceName);
        }

        if (inch > 0) {
            text += QString("（%1 英寸）").arg(QString::number(inch, 'f', 1));
        }

        if (!text.isEmpty() && !result.contains(text))
            result << text;

        VariantClear(&vtInstanceName);
        VariantClear(&vtManufacturerName);
        VariantClear(&vtUserFriendlyName);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);

    if (result.isEmpty()) {
        result = queryStringList("Win32_DesktopMonitor", "Name");
    }

    return result;
}

QStringList HardwareInfo::diskDrives()
{
    QStringList result;

    WmiContext ctx;
    if (!initializeWMI(ctx))
        return QStringList();

    IWbemLocator* pLoc = ctx.loc;
    IWbemServices* pSvc = ctx.svc;

    const QString query = "SELECT Model, Size FROM Win32_DiskDrive";

    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(reinterpret_cast<const wchar_t*>(query.utf16())),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        cleanupWMI(ctx);
        return result;
    }

    IWbemClassObject* pObj = nullptr;
    ULONG returned = 0;

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned);
        if (FAILED(hr) || returned == 0)
            break;

        VARIANT vtModel, vtSize;
        VariantInit(&vtModel);
        VariantInit(&vtSize);

        pObj->Get(L"Model", 0, &vtModel, nullptr, nullptr);
        pObj->Get(L"Size", 0, &vtSize, nullptr, nullptr);

        const QString model = variantToQString(vtModel);
        const QString sizeStr = variantToQString(vtSize);

        QString text = model;
        bool ok = false;
        qulonglong bytes = sizeStr.toULongLong(&ok);
        if (ok)
            text += " (" + formatBytes(bytes) + ")";

        if (!text.isEmpty() && !result.contains(text))
            result << text;

        VariantClear(&vtModel);
        VariantClear(&vtSize);
        pObj->Release();
    }

    pEnumerator->Release();
    cleanupWMI(ctx);
    return result;
}

QStringList HardwareInfo::soundCards()
{
    return queryStringList("Win32_SoundDevice", "Name");
}

QStringList HardwareInfo::networkCards()
{
    return queryStringList("Win32_NetworkAdapter", "Name", "PhysicalAdapter=True");
}

QStringList HardwareInfo::graphicsCards()
{
    QStringList result;
    QSet<QString> uniqueSet;

    // 用 DXGI 读取更可靠的专用显存
    const QMap<QString, QString> dxgiMemoryMap = queryGpuMemoryByDxgi();

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needUninit = SUCCEEDED(hres);

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
        if (needUninit) CoUninitialize();
        return result;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&pLoc)
    );

    if (FAILED(hres) || !pLoc) {
        if (needUninit) CoUninitialize();
        return result;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres) || !pSvc) {
        pLoc->Release();
        if (needUninit) CoUninitialize();
        return result;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        if (needUninit) CoUninitialize();
        return result;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Name, DriverVersion, VideoProcessor FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (SUCCEEDED(hres) && pEnumerator) {
        IWbemClassObject* pObj = NULL;
        ULONG uReturn = 0;

        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn);
            if (uReturn == 0) {
                break;
            }

            QString name, driverVersion, videoProcessor;

            VARIANT vtProp;
            VariantInit(&vtProp);

            // Name
            if (SUCCEEDED(pObj->Get(L"Name", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                name = QString::fromWCharArray(vtProp.bstrVal).trimmed();
            }
            VariantClear(&vtProp);

            // DriverVersion
            VariantInit(&vtProp);
            if (SUCCEEDED(pObj->Get(L"DriverVersion", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                driverVersion = QString::fromWCharArray(vtProp.bstrVal).trimmed();
            }
            VariantClear(&vtProp);

            // VideoProcessor
            VariantInit(&vtProp);
            if (SUCCEEDED(pObj->Get(L"VideoProcessor", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                videoProcessor = QString::fromWCharArray(vtProp.bstrVal).trimmed();
            }
            VariantClear(&vtProp);

            QString line = name;
            QStringList extra;

            if (!videoProcessor.isEmpty() && videoProcessor != name)
                extra << videoProcessor;

            // 优先用 DXGI 名称精确匹配显存
            if (dxgiMemoryMap.contains(name)) {
                extra << QString("显存: %1").arg(dxgiMemoryMap.value(name));
            }
            else {
                // 名称不完全一致时，做一次简单模糊匹配
                for (auto it = dxgiMemoryMap.constBegin(); it != dxgiMemoryMap.constEnd(); ++it) {
                    const QString dxgiName = it.key();
                    if (dxgiName.contains(name, Qt::CaseInsensitive) ||
                        name.contains(dxgiName, Qt::CaseInsensitive)) {
                        extra << QString("显存: %1").arg(it.value());
                        break;
                    }
                }
            }

            if (!driverVersion.isEmpty())
                extra << QString("驱动版本: %1").arg(driverVersion);

            if (!extra.isEmpty()) {
                line += QString("（%1）").arg(extra.join("，"));
            }

            if (!line.isEmpty() && !uniqueSet.contains(line)) {
                uniqueSet.insert(line);
                result << line;
            }

            pObj->Release();
        }

        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    if (needUninit) CoUninitialize();

    return result;
}