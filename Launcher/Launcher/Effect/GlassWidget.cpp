#include "GlassWidget.h"
#include <QPainter>
#include <QPainterPath>

GlassWidget::GlassWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void GlassWidget::setBackground(const QImage& img)
{
    if (img.isNull())
        return;

    m_sourceImage = img;
    m_blurImage = blurImage(img, m_blurRadius);

    update();
}

QImage GlassWidget::blurImage(const QImage& src, int radius)
{
    if (src.isNull())
        return src;

    QImage img = src.convertToFormat(QImage::Format_ARGB32);

    for (int i = 0; i < radius; ++i)
    {
        img = img.scaled(
            img.width() / 2,
            img.height() / 2,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);

        img = img.scaled(
            src.size(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
    }

    return img;
}

void GlassWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();

    QPainterPath path;
    path.addRoundedRect(r, 20, 20);
    painter.setClipPath(path);

    // 半透明玻璃层
    painter.fillRect(r, m_glassColor);

    // 顶部高光
    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    grad.setColorAt(0, QColor(255, 255, 255, 80));
    grad.setColorAt(1, QColor(255, 255, 255, 10));

    painter.fillRect(r, grad);

    // 边框
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1));
    painter.drawRoundedRect(r.adjusted(0, 0, -1, -1), 20, 20);
}

void GlassWidget::setBlurRadius(int radius)
{
    m_blurRadius = qMax(0, radius);

    if (!m_sourceImage.isNull())
    {
        m_blurImage = blurImage(m_sourceImage, m_blurRadius);
        update();
    }
}

void GlassWidget::setGlassColor(const QColor& color)
{
    m_glassColor = color;
    update();
}