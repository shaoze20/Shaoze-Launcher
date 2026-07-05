#pragma once
#include <QPushButton>
#include <QPainterPath>

class OutlineButton : public QPushButton
{
    Q_OBJECT

public:
    explicit OutlineButton(const QString& text, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_hovered = false;
};