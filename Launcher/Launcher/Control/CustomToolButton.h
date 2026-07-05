#pragma once

#ifndef CLOSEBUTTON_H
#define CLOSEBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QWindow>
#include <QScreen>

class CustomToolButton : public QPushButton {
    Q_OBJECT
public:
    explicit CustomToolButton(QWidget* parent = nullptr);

    // 外部调用此函数传入图片
    void setIconPixmap(const QPixmap& pix);
    void setIconSize(const QSize& size);
    void setDrawBackground(bool enable);
    void setBackgroundColors(const QColor& normal, const QColor& hover, const QColor& pressed);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QSize m_iconSize = QSize(16, 16);
    bool isHovered = false;
    bool isPressed = false;
    QPixmap m_iconPix;

    bool m_drawBackground = false;

    // 按钮颜色
    QColor m_normalColor = Qt::transparent;
    QColor m_hoverColor = QColor(255, 255, 255, 100);
    QColor m_pressedColor = QColor(220, 220, 220, 150);
};

#endif