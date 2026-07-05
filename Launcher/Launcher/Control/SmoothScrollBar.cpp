#include "SmoothScrollBar.h"
#include <QWheelEvent>
#include <QtMath>

SmoothScrollBar::SmoothScrollBar(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(orientation, parent),
    m_targetValue(value()),
    m_currentValue(value()),
    m_wheelStep(0.5),
    m_duration(300)
{
    setStyleSheet(R"(
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 4px 2px 4px 2px;
        }

        QScrollBar::handle:vertical {
            background: rgba(120, 120, 120, 90);   /* 更淡 */
            border-radius: 3px;
            min-height: 40px;
        }

        QScrollBar::handle:vertical:hover {
            background: rgba(100, 100, 100, 130);  /* hover 也稍微淡 */
        }

        QScrollBar::handle:vertical:pressed {
            background: rgba(80, 80, 80, 160);
        }


        QScrollBar:horizontal {
            background: transparent;
            height: 10px;
            margin: 2px 4px 2px 4px;
        }

        QScrollBar::handle:horizontal {
            background: rgba(120, 120, 120, 90);
            border-radius: 3px;
            min-width: 40px;
        }

        QScrollBar::handle:horizontal:hover {
            background: rgba(100, 100, 100, 130);
        }

        QScrollBar::handle:horizontal:pressed {
            background: rgba(80, 80, 80, 160);
        }


        QScrollBar::add-line,
        QScrollBar::sub-line {
            background: transparent;
            border: none;
            width: 0;
            height: 0;
        }

        QScrollBar::add-page,
        QScrollBar::sub-page {
            background: transparent;
        }
      )");

    m_currentValue = value();
    m_targetValue = value();

    int interval = 1000 / screen()->refreshRate();
    m_timer.setInterval(interval);
    connect(&m_timer, &QTimer::timeout, this, &SmoothScrollBar::updateAnimation);
}

void SmoothScrollBar::showEvent(QShowEvent* event)
{
    QScrollBar::showEvent(event);
    m_currentValue = value();
    m_targetValue = value();
}

void SmoothScrollBar::setScrollDuration(int ms)
{
    m_duration = ms;
}

void SmoothScrollBar::setWheelStep(qreal step)
{
    m_wheelStep = step;
}

void SmoothScrollBar::setValue(int v)
{
    v = qBound(minimum(), v, maximum());

    if (v == qRound(m_targetValue))
        return;

    m_targetValue = v;

    startAnimation();
}

void SmoothScrollBar::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    if (delta == 0) return;

    if (!m_timer.isActive()) {
        m_currentValue = value();
        m_targetValue = value();
    }

    qreal step = delta * m_wheelStep;
    m_targetValue -= step;
    m_targetValue = qBound(qreal(minimum()), m_targetValue, qreal(maximum()));

    startAnimation();
    event->accept();
}

void SmoothScrollBar::startAnimation()
{
    if (!m_timer.isActive())
        m_timer.start();
}

void SmoothScrollBar::updateAnimation()
{
    qreal t = 0.2;
    qreal diff = m_targetValue - m_currentValue;

    if (qAbs(diff) < 1.0) {
        m_currentValue = m_targetValue;
        QScrollBar::setValue(qRound(m_currentValue));
        m_timer.stop();
        return;
    }

    m_currentValue += diff * t;
    QScrollBar::setValue(qRound(m_currentValue));
}