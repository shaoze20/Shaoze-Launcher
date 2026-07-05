#include "CustomCheckBox.h"
#include <QPainter>
#include <QEvent>

CustomCheckBox::CustomCheckBox(const QString& text, QWidget* parent)
    : QAbstractButton(parent)
{
    setText(text);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(m_boxSize + 10, m_boxSize + 10);

    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_animation, &QVariantAnimation::valueChanged, [this](const QVariant& value) {
        setAnimValue(value.toReal());
        });
}

void CustomCheckBox::nextCheckState()
{
    QAbstractButton::nextCheckState();
    bool checked = isChecked();

    m_animation->stop();
    m_animation->setStartValue(m_animValue);
    m_animation->setEndValue(checked ? 1.0 : 0.0);
    m_animation->start();
}

void CustomCheckBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 动态计算方框大小
    int boxSize = height() * 0.6;
    QRect boxRect(5, (height() - boxSize) / 2, boxSize, boxSize);
    QRect textRect = rect().adjusted(boxSize + 10, 0, 0, 0);
    qreal borderRadius = boxSize / 4.0;

    // 绘制边框
    painter.setPen(QPen(QColor(200, 200, 200), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(boxRect, borderRadius, borderRadius);

    // 动态背景填充
    if (m_animValue > 0) {
        painter.save();
        painter.setOpacity(m_animValue);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 120, 215));

        // 使用缩放逻辑，使其随 size 变化
        painter.translate(boxRect.center());
        painter.scale(0.8 + 0.2 * m_animValue, 0.8 + 0.2 * m_animValue);
        painter.translate(-boxRect.center());

        painter.drawRoundedRect(boxRect, borderRadius, borderRadius);
        painter.restore();
    }

    // 动态绘制勾勾
    if (m_animValue > 0.4) {
        // 线条粗细
        painter.setPen(QPen(Qt::white, boxSize * 0.12, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        // 比例坐标
        QPointF p1(boxRect.left() + boxSize * 0.25, boxRect.center().y());
        QPointF p2(boxRect.left() + boxSize * 0.45, boxRect.bottom() - boxSize * 0.3);
        QPointF p3(boxRect.right() - boxSize * 0.25, boxRect.top() + boxSize * 0.3);

        QPainterPath path;
        path.moveTo(p1);
        qreal t = (m_animValue - 0.4) / 0.6;
        if (t < 0.5) {
            path.lineTo(p1 + (p2 - p1) * (t / 0.5));
        }
        else {
            path.lineTo(p2);
            path.lineTo(p2 + (p3 - p2) * ((t - 0.5) / 0.5));
        }
        painter.drawPath(path);
    }

    // 字体大小与垂直居中
    QFont font = this->font();
    font.setPixelSize(height() * 0.6);
    painter.setFont(font);
    QFontMetrics fm(font);
    int textHeight = fm.height();
    int boxCenterY = height() / 2;
    int textTop = boxCenterY - (textHeight / 2) - 1;
    QRect alignedTextRect(textRect.left(), textTop, textRect.width(), textHeight);
    painter.setPen(QColor(60, 60, 60));
    painter.drawText(alignedTextRect, Qt::AlignLeft | Qt::AlignVCenter, text());
}

void CustomCheckBox::setChecked(bool checked)
{
    if (isChecked() == checked)
        return;

    QAbstractButton::setChecked(checked);

    m_animation->stop();
    m_animValue = checked ? 1.0 : 0.0;
    update();
}