#include "CustomButton.h"
#include <QPainter>

CustomButton::CustomButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent), m_opacity(0.6)
{
    // 初始化动画
    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_animation, &QVariantAnimation::valueChanged, [this](const QVariant& value) {
        setOpacity(value.toReal());
        });

    setMinimumSize(120, 40);
}

void CustomButton::enterEvent(QEnterEvent* event)
{
    m_animation->stop();
    m_animation->setStartValue(m_opacity);
    m_animation->setEndValue(1.0);
    m_animation->start();
    QPushButton::enterEvent(event);
}

void CustomButton::leaveEvent(QEvent* event)
{
    m_animation->stop();
    m_animation->setStartValue(m_opacity);
    m_animation->setEndValue(0.6);
    m_animation->start();
    QPushButton::leaveEvent(event);
}

void CustomButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(m_opacity);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_bgColor);
    painter.drawRoundedRect(rect(), m_radius, m_radius);
    painter.setOpacity(1.0);

    painter.setPen(m_textColor);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(m_fontSize);
    painter.setFont(font);

    // 居中绘制按钮文本
    painter.drawText(rect(), Qt::AlignCenter, text());
}

void CustomButton::setRadius(int r)
{
    m_radius = r;
    update();
}

int CustomButton::radius() const
{
    return m_radius;
}

void CustomButton::setBackgroundColor(const QColor& color)
{
    m_bgColor = color;
    update();
}

QColor CustomButton::backgroundColor() const
{
    return m_bgColor;
}

void CustomButton::setTextColor(const QColor& color)
{
    m_textColor = color;
    update();
}

QColor CustomButton::textColor() const
{
    return m_textColor;
}

void CustomButton::setFontSize(int size)
{
    m_fontSize = size;
    update();
}

int CustomButton::fontSize() const
{
    return m_fontSize;
}