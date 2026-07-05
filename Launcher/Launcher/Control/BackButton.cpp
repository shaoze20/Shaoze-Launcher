#include "BackButton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QEnterEvent>

BackButton::BackButton(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(m_diameter, m_diameter);
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
}

void BackButton::setDiameter(int diameter)
{
    if (diameter < 24)
        diameter = 24;

    m_diameter = diameter;
    setFixedSize(m_diameter, m_diameter);
    update();
}

int BackButton::diameter() const
{
    return m_diameter;
}

void BackButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF r = rect().adjusted(1.0, 1.0, -1.0, -1.0);

    // 背景色
    QColor bgColor = Qt::white;
    QColor borderColor(180, 180, 180);
    QColor arrowColor(90, 90, 90);

    if (m_hovered) {
        borderColor = QColor(150, 150, 150);
        arrowColor = QColor(60, 60, 60);
    }

    if (m_pressed) {
        bgColor = QColor(245, 245, 245);
        borderColor = QColor(130, 130, 130);
        arrowColor = QColor(40, 40, 40);
    }

    // 画圆形背景
    painter.setPen(QPen(borderColor, 1.5));
    painter.setBrush(bgColor);
    painter.drawEllipse(r);

    // 画返回箭头
    QPen pen(arrowColor, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    const qreal w = width();
    const qreal h = height();

    qreal offsetX = -w * 0.06;
    QPointF p1(w * 0.58 + offsetX, h * 0.30);
    QPointF p2(w * 0.38 + offsetX, h * 0.50);
    QPointF p3(w * 0.58 + offsetX, h * 0.70);

    painter.drawLine(p1, p2);
    painter.drawLine(p2, p3);

    painter.drawLine(QPointF(w * 0.40 + offsetX, h * 0.50),
        QPointF(w * 0.70 + offsetX, h * 0.50));
}

void BackButton::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);
    m_hovered = true;
    update();
}

void BackButton::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    m_hovered = false;
    m_pressed = false;
    update();
}

void BackButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
    QWidget::mousePressEvent(event);
}

void BackButton::mouseReleaseEvent(QMouseEvent* event)
{
    bool wasPressed = m_pressed;
    m_pressed = false;
    update();

    if (wasPressed && rect().contains(event->pos()) && event->button() == Qt::LeftButton) {
        emit clicked();
    }

    QWidget::mouseReleaseEvent(event);
}