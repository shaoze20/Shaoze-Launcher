#include "OutlineButton.h"
#include <QPainter>

OutlineButton::OutlineButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFlat(true);
}

void OutlineButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void OutlineButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QPushButton::leaveEvent(event);
}

void OutlineButton::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_hovered) {
        m_hovered = true;
        update();
    }
    QPushButton::mouseMoveEvent(event);
}

void OutlineButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF r = rect().adjusted(1, 1, -1, -1);

    QPainterPath path;
    path.addRoundedRect(r, 8, 8);

    //状态
    QColor textColor(255, 255, 255);
    QColor bgColor(255, 255, 255, 0); // 默认透明

    if (m_hovered) {
        bgColor = QColor(255, 255, 255, 40);
    }

    if (isDown()) {
        bgColor = QColor(255, 255, 255, 50);
        textColor = QColor(230, 230, 230);
    }

    // 背景
    if (bgColor.alpha() > 0) {
        painter.fillPath(path, bgColor);
    }

    // 文字
    painter.setPen(textColor);
    QFont f = font();
    f.setPointSize(10);
    painter.setFont(f);
    QRect textRect = rect().adjusted(12, 0, -12, 0); // 左右留边距
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
}