#pragma once

#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QWidget>
#include <QWindow>
#include <QPixmapCache>

class Globals{
public:
    static void setSystemGlassEnabled(QWidget* widget, bool enabled);
    static void printMemoryUsage(const QString& tag);
    static QPixmap getRoundedPixmap(const QPixmap& imagePath);
    static QImage blurImage(const QImage& source, qreal radius = 10.0);
    static QPixmap makeHighDpiPixmap(const QPixmap& src, const QSize& logicalSize, qreal dpr);
};