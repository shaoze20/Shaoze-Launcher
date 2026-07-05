#include "CapsuleCheckBox.h"
#include <QPainter>
#include <QMouseEvent>

CapsuleCheckBox::CapsuleCheckBox(const QString& text, QWidget* parent)
    : QWidget(parent), m_text(text)
{
    m_frameTimer = new QTimer(this);
    m_frameTimer->setTimerType(Qt::PreciseTimer);

    int refreshRate = 60;
    if (screen() && screen()->refreshRate() > 1) {
        refreshRate = int(screen()->refreshRate());
    }

    int interval = 1000 / refreshRate;
    m_frameTimer->setInterval(interval);

    connect(m_frameTimer, &QTimer::timeout, this, [this]() {
        if (!m_animating) return;

        qreal t = qreal(m_elapsedTimer.elapsed()) / qreal(m_animDuration);
        if (t > 1.0) t = 1.0;

        // OutCubic
        qreal k = 1.0 - qPow(1.0 - t, 3.0);

        m_offset = m_animStart + (m_animEnd - m_animStart) * k;
        update();

        if (t >= 1.0) {
            m_offset = m_animEnd;
            m_animating = false;
            m_frameTimer->stop();
            update();
        }
        });
}

void CapsuleCheckBox::setText(const QString& text)
{
    m_text = text;
    update();
}

bool CapsuleCheckBox::isChecked() const
{
    return m_checked;
}

void CapsuleCheckBox::setChecked(bool checked)
{
    if (m_checked == checked && !m_animating) {
        return;
    }

    m_checked = checked;

    m_animStart = m_offset;
    m_animEnd = checked ? 1.0 : 0.0;

    m_animating = true;
    m_elapsedTimer.restart();
    m_frameTimer->start();

    emit toggled(m_checked);
}

void CapsuleCheckBox::mousePressEvent(QMouseEvent*)
{
    setChecked(!m_checked);
}

qreal CapsuleCheckBox::offset() const
{
    return m_offset;
}

void CapsuleCheckBox::setOffset(qreal value)
{
    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;

    m_offset = value;
    update();
}

void CapsuleCheckBox::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int w = width();
    const int h = height();

    if (w <= 0 || h <= 0)
        return;

    // 控件
    const qreal sideMargin = qMax(2.0, h * 0.06);
    const qreal spacing = qMax(4.0, h * 0.12);
    const qreal switchHeight = h - sideMargin * 2.0;
    const qreal switchWidth = switchHeight * 2.0;

    QRectF switchRect(
        sideMargin,
        sideMargin,
        switchWidth,
        switchHeight
    );

    // 背景颜色插值
    const QColor offColor(255, 105, 105);
    const QColor onColor(0, 200, 120);

    QColor bgColor(
        int(offColor.red() + (onColor.red() - offColor.red()) * m_offset),
        int(offColor.green() + (onColor.green() - offColor.green()) * m_offset),
        int(offColor.blue() + (onColor.blue() - offColor.blue()) * m_offset)
    );

    // 外层背景 + 边框
    QPen borderPen;
    borderPen.setWidthF(1.2);
    borderPen.setColor(QColor(200, 200, 200));

    p.setPen(borderPen);
    p.setBrush(bgColor);

    p.drawRoundedRect(
        switchRect,
        switchHeight / 2.0,
        switchHeight / 2.0
    );

    // 滑块
    const qreal innerMargin = switchHeight * 0.12;
    const qreal diameter = switchHeight - innerMargin * 2.0;

    const qreal travel = switchRect.width() - diameter - innerMargin * 2.0;
    const qreal x = switchRect.x() + innerMargin + travel * m_offset;
    const qreal y = switchRect.y() + innerMargin;

    QRectF circleRect(x, y, diameter, diameter);

    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawEllipse(circleRect);

    // 文字区域
    QRectF textRect(
        switchRect.right() + spacing,
        0,
        w - (switchRect.right() + spacing) - sideMargin,
        h
    );

    QFont font = this->font();
    font.setPixelSize(int(h * 0.65));   // 字号
    p.setFont(font);
    p.setPen(palette().color(QPalette::WindowText));
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_text);
}