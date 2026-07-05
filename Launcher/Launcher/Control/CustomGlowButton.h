#pragma once

#include <QPushButton>
#include <QColor>
#include <QLabel>
#include <QTimer>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QWindow>

#include "../Globals/Globals.h"

class CustomGlowButton : public QPushButton
{
    Q_OBJECT
        Q_PROPERTY(qreal borderOpacity READ borderOpacity WRITE setBorderOpacity)
        Q_PROPERTY(QSize iconAnimSize READ iconAnimSize WRITE setIconAnimSize)

public:
    explicit CustomGlowButton(QWidget* parent = nullptr);

    void setIconPixmap(const QPixmap& pix);
    void setIconSize(const QSize& size);
    void setTooltipText(const QString& text);

    qreal borderOpacity() const { return m_borderOpacity; }
    void setBorderOpacity(qreal value);

    QSize iconAnimSize() const { return m_iconAnimSize; }
    void setIconAnimSize(const QSize& size){
        m_iconAnimSize = size;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QSize m_iconAnimSize;
    QPixmap m_icon;
    QPixmap m_scaledIcon;
    QSize m_iconSize = QSize(20, 20);

    bool m_hovered = false;
    QColor m_hoverColor = QColor(200, 200, 200, 120);;

    qreal m_borderOpacity = 0.0;
    QPropertyAnimation* m_borderAnim = nullptr;
    QPropertyAnimation* m_iconAnim = nullptr;

    QString m_tooltipText;
    QWidget* m_tooltipWidget = nullptr;
    QTimer* m_tooltipTimer = nullptr;
    QPoint m_tooltipTargetPos;
};