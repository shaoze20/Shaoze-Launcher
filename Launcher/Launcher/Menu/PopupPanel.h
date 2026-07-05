#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

class PopupPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PopupPanel(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};