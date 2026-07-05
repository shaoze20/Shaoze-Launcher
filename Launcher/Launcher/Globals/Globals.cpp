#include "Globals.h"

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <objidl.h>
#include <winuser.h>
#include <psapi.h>
#pragma comment (lib,"User32.lib")
#pragma comment (lib,"dwmapi.lib")
#endif

void Globals::setSystemGlassEnabled(QWidget* widget, bool enabled)
{
#ifdef Q_OS_WIN
    if (!widget) return;

    HWND hwnd = (HWND)widget->winId();
    if (!hwnd) return;

    // 设置透明背景
    widget->setAttribute(Qt::WA_TranslucentBackground);

    // 延伸边框实现阴影
    const MARGINS shadow_state = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &shadow_state);

    // Win11 暗色模式主题
    int darkMode = 0;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(int));

    // 设置背景材质类型
    int backdropType = enabled ? 3 : 2;
    DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(int));

    // 允许标题栏功能
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME);
    ::SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    ::InvalidateRect(hwnd, NULL, TRUE);
    ::UpdateWindow(hwnd);
#else
    Q_UNUSED(widget);
    Q_UNUSED(enabled);
#endif
}

// 查看内存占用
void Globals::printMemoryUsage(const QString& tag)
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
        sizeof(pmc)))
    {
        const double workingSetMB = pmc.WorkingSetSize / 1024.0 / 1024.0;
        const double privateMB = pmc.PrivateUsage / 1024.0 / 1024.0;

        qDebug().noquote()
            << QString("[%1] WorkingSet=%2 MB, Private=%3 MB")
            .arg(tag)
            .arg(QString::number(workingSetMB, 'f', 1))
            .arg(QString::number(privateMB, 'f', 1));
    }
}

//裁剪圆形头像
QPixmap Globals::getRoundedPixmap(const QPixmap& src)
{
    if (src.isNull()) return QPixmap();

    int srcWidth = src.width();
    int srcHeight = src.height();
    int minSide = qMin(srcWidth, srcHeight);

    const int maxSide = 256;
    int targetSide = qMin(minSide, maxSide);

    static QHash<QString, QPixmap> cache;
    QString key = QString("%1_%2").arg(src.cacheKey()).arg(targetSide);

    auto it = cache.find(key);
    if (it != cache.end()) {
        return it.value();
    }

    QRect sourceRect(
        (srcWidth - minSide) / 2,
        (srcHeight - minSide) / 2,
        minSide,
        minSide
    );

    QPixmap square = src.copy(sourceRect);

    if (minSide > maxSide) {
        square = square.scaled(
            targetSide,
            targetSide,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        );
    }

    QPixmap target(targetSide, targetSide);
    target.fill(Qt::transparent);

    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addEllipse(0, 0, targetSide, targetSide);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, square);

    cache.insert(key, target);
    return target;
}

QImage Globals::blurImage(const QImage& source, qreal radius)
{
    if (source.isNull())
        return QImage();

    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(source));
    QGraphicsBlurEffect blur;
    blur.setBlurRadius(radius);
    item.setGraphicsEffect(&blur);
    scene.addItem(&item);
    QImage result(source.size(), QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    scene.render(&painter, QRectF(), QRectF(0, 0, source.width(), source.height()));
    return result;
}

// 高dpi图片
QPixmap Globals::makeHighDpiPixmap(const QPixmap& src, const QSize& logicalSize, qreal dpr)
{
    if (src.isNull() || logicalSize.isEmpty()) {
        return QPixmap();
    }

    QSize physicalSize = QSize(
        qRound(logicalSize.width() * dpr),
        qRound(logicalSize.height() * dpr)
    );

    QPixmap scaled = src.scaled(
        physicalSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    scaled.setDevicePixelRatio(dpr);
    return scaled;
}