#pragma once
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>

class GlassClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlassClockWidget(QWidget* parent = nullptr);

    void setCornerRadius(int radius);
    void setBlurStrength(int strength);
    void setUse24Hour(bool use24Hour);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateTimeText();
    void drawGlassBackground(QPainter& p);
    void drawTimeText(QPainter& p);
    void drawBlurLayers(QPainter& p, const QRectF& rect, int radius);

private:
    QTimer* m_timer = nullptr; 
    QString m_periodText;

    QString m_timeText;
    QString m_dateText;

    int m_cornerRadius = 24;
    int m_blurStrength = 18;
    bool m_use24Hour = true;
};