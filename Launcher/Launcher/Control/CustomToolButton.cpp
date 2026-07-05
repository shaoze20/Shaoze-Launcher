#include "CustomToolButton.h"
#include <QPainter>

CustomToolButton::CustomToolButton(QWidget* parent) : QPushButton(parent) {
    setFixedSize(40, 40);  // 设置默认大小
}

void CustomToolButton::setIconPixmap(const QPixmap& pix) {
    m_iconPix = pix;
    update();
}

void CustomToolButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 清理背景
    if (m_drawBackground) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // 背景颜色
    QColor bgColor = m_normalColor;
    if (isPressed) {
        bgColor = m_pressedColor;
    }
    else if (isHovered) {
        bgColor = m_hoverColor;
    }

    // 绘制背景
    if (bgColor != Qt::transparent) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(bgColor);
        QRectF r = rect().adjusted(1, 1, -1, -1);
        painter.drawRoundedRect(r, 4, 4);
    }

    // 绘制图标
    if (!m_iconPix.isNull()) {
        QSize logicalSize = m_iconSize.isValid() ? m_iconSize : QSize(16, 16);

        qreal dpr = devicePixelRatioF();
        if (windowHandle() && windowHandle()->screen()) {
            dpr = windowHandle()->screen()->devicePixelRatio();
        }

        QSize physicalSize(
            qRound(logicalSize.width() * dpr),
            qRound(logicalSize.height() * dpr)
        );

        QPixmap hdpiPixmap = m_iconPix.scaled(
            physicalSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        hdpiPixmap.setDevicePixelRatio(dpr);

        QSize drawSize = hdpiPixmap.deviceIndependentSize().toSize();

        QPoint center = rect().center();
        QRectF iconRect(
            center.x() - drawSize.width() / 2.0 + 1,
            center.y() - drawSize.height() / 2.0,
            drawSize.width(),
            drawSize.height()
        );

        painter.drawPixmap(iconRect.topLeft(), hdpiPixmap);
    }
}

void CustomToolButton::enterEvent(QEnterEvent* event) {
    isHovered = true;
    update();
    QPushButton::enterEvent(event);
}

void CustomToolButton::leaveEvent(QEvent* event) {
    isHovered = false;
    update();
    QPushButton::leaveEvent(event);
}

void CustomToolButton::mousePressEvent(QMouseEvent* event) {
    isPressed = true;
    update();
    QPushButton::mousePressEvent(event);
}

void CustomToolButton::mouseReleaseEvent(QMouseEvent* event) {
    isPressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}

void CustomToolButton::setIconSize(const QSize& size)
{
    m_iconSize = size;
    update();
}

void CustomToolButton::setDrawBackground(bool enable)
{
    m_drawBackground = enable;
    update();
}

void CustomToolButton::setBackgroundColors(const QColor& normal,const QColor& hover,const QColor& pressed)
{
    m_normalColor = normal;
    m_hoverColor = hover;
    m_pressedColor = pressed;
    update();
}