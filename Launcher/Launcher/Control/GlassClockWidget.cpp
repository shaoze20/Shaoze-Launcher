#include "GlassClockWidget.h"

GlassClockWidget::GlassClockWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);

    connect(m_timer, &QTimer::timeout, this, [this]() {
        updateTimeText();
        update();
        });

    updateTimeText();
    m_timer->start();

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(35);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadow);
}

void GlassClockWidget::setCornerRadius(int radius)
{
    m_cornerRadius = qMax(0, radius);
    update();
}

void GlassClockWidget::setBlurStrength(int strength)
{
    m_blurStrength = qMax(0, strength);
    update();
}

void GlassClockWidget::setUse24Hour(bool use24Hour)
{
    m_use24Hour = use24Hour;
    updateTimeText();
    update();
}

void GlassClockWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void GlassClockWidget::updateTimeText()
{
    const QDateTime now = QDateTime::currentDateTime();
    const int hour = now.time().hour();

    if (m_use24Hour) {
        m_timeText = now.time().toString("HH:mm:ss");
    }
    else {
        m_timeText = now.time().toString("hh:mm:ss AP");
    }

    m_dateText = now.date().toString("yyyy-MM-dd dddd");

    if (hour >= 0 && hour < 5) {
        m_periodText = QStringLiteral("凌晨");
    }
    else if (hour >= 5 && hour < 8) {
        m_periodText = QStringLiteral("早上");
    }
    else if (hour >= 8 && hour < 11) {
        m_periodText = QStringLiteral("上午");
    }
    else if (hour >= 11 && hour < 13) {
        m_periodText = QStringLiteral("中午");
    }
    else if (hour >= 13 && hour < 18) {
        m_periodText = QStringLiteral("下午");
    }
    else if (hour >= 18 && hour < 20) {
        m_periodText = QStringLiteral("傍晚");
    }
    else {
        m_periodText = QStringLiteral("晚上");
    }
}

void GlassClockWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    drawGlassBackground(p);
    drawTimeText(p);
}

void GlassClockWidget::drawGlassBackground(QPainter& p)
{
    QRectF cardRect = rect().adjusted(12.5, 12.5, -12.5, -12.5);

    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addRoundedRect(cardRect, m_cornerRadius, m_cornerRadius);

    // 半透明玻璃背景
    QLinearGradient bg(cardRect.topLeft(), cardRect.bottomRight());
    bg.setColorAt(0.0, QColor(255, 255, 255, 180));
    bg.setColorAt(1.0, QColor(245, 245, 245, 140));

    p.fillPath(path, bg);
    p.setClipPath(path);
    p.setClipping(false);
    p.restore();
}

void GlassClockWidget::drawBlurLayers(QPainter& p, const QRectF& rect, int radius)
{
    const int layers = qMax(6, radius / 2);
    for (int i = layers; i >= 1; --i) {
        const qreal offset = i * 0.8;
        QRectF r = rect.adjusted(offset, offset, -offset, -offset);

        int alpha = qMax(4, 20 - i);
        QColor c(255, 255, 255, alpha);

        QPainterPath path;
        path.addRoundedRect(r, m_cornerRadius, m_cornerRadius);
        p.fillPath(path, c);
    }


    {
        QLinearGradient cold(rect.topLeft(), rect.bottomRight());
        cold.setColorAt(0.0, QColor(180, 220, 255, 22));
        cold.setColorAt(0.5, QColor(200, 235, 255, 10));
        cold.setColorAt(1.0, QColor(220, 240, 255, 6));

        QPainterPath path;
        path.addRoundedRect(rect, m_cornerRadius, m_cornerRadius);
        p.fillPath(path, cold);
    }
}

void GlassClockWidget::drawTimeText(QPainter& p)
{
    QRectF rect = this->rect().adjusted(20, 20, -20, -20);

    QFont timeFont("Bahnschrift");
    timeFont.setPixelSize(qMax(34, height() / 3));
    timeFont.setBold(true);

    QFont dateFont("Microsoft YaHei");
    dateFont.setPixelSize(qMax(14, height() / 10));
    dateFont.setBold(false);

    QFont periodFont("Bahnschrift");
    periodFont.setPixelSize(qMax(14, height() / 12));
    periodFont.setBold(true);

    QFontMetrics timeFm(timeFont);
    QFontMetrics dateFm(dateFont);
    QFontMetrics periodFm(periodFont);

    const int spacing = 0;
    const int totalH = timeFm.height() + spacing + dateFm.height();

    QRectF timeRect(
        rect.left(),
        rect.top() + (rect.height() - totalH) / 2.0,
        rect.width(),
        timeFm.height()
    );

    QRectF dateRect(
        rect.left(),
        timeRect.bottom() + spacing,
        rect.width(),
        dateFm.height()
    );

    QRectF periodRect(
        timeRect.left() + rect.width() * 0.05,                     // 时间左侧
        timeRect.top() - periodFm.height() * 0.6,                  // 时间左上
        periodFm.horizontalAdvance(m_periodText) + 16,
        periodFm.height() + 4
    );

    // 时段阴影
    p.setFont(periodFont);
    p.setPen(QColor(0, 0, 0, 18));
    p.drawText(periodRect.translated(0, 1), Qt::AlignLeft | Qt::AlignVCenter, m_periodText);

    // 时段主文字
    p.setPen(QColor(80, 80, 80));
    p.drawText(periodRect, Qt::AlignLeft | Qt::AlignVCenter, m_periodText);

    // 时间阴影
    p.setFont(timeFont);
    p.setPen(QColor(0, 0, 0, 20));
    p.drawText(timeRect.translated(0, 1), Qt::AlignCenter, m_timeText);

    // 时间主文字
    p.setPen(QColor(25, 25, 25));
    p.drawText(timeRect, Qt::AlignCenter, m_timeText);

    // 日期阴影
    p.setFont(dateFont);
    p.setPen(QColor(0, 0, 0, 12));
    p.drawText(dateRect.translated(0, 1), Qt::AlignCenter, m_dateText);

    // 日期主文字
    p.setPen(QColor(90, 90, 90));
    p.drawText(dateRect, Qt::AlignCenter, m_dateText);
}