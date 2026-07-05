#pragma once
#include <QScrollBar>
#include <QTimer>

class SmoothScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit SmoothScrollBar(Qt::Orientation orientation, QWidget* parent = nullptr);

    void setScrollDuration(int ms);
    void setWheelStep(qreal step);

    void setValue(int v);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void startAnimation();
    void updateAnimation();

private:
    qreal m_targetValue;
    qreal m_currentValue;
    qreal m_wheelStep;
    int m_duration;
    QTimer m_timer;
};