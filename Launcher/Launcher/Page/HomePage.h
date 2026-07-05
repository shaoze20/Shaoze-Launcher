#pragma once

#include <QLabel>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath> 

#include "../Control/GlassClockWidget.h"
#include "../Design/DesignSystem.h"
#include "../Style/StyleSheet.h"

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* m_overlay = nullptr;
    GlassClockWidget* m_clock = nullptr;

    QPixmap m_bgOriginal;
};