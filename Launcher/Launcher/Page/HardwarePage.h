#pragma once
#ifndef HARDWAREPAGE_H
#define HARDWAREPAGE_H

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPainter>
#include <QFrame>
#include <QTimer>

#include "../WMI/HardwareInfo.h"
#include "../Control/InfoCard.h"
#include "../Control/SmoothScrollBar.h"
#include "../Style/StyleSheet.h"
#include "../Design/DesignSystem.h"
#include "../Control/CustomTabWidget.h"

class HardwarePage : public QWidget
{
    Q_OBJECT

public:
    explicit HardwarePage(QWidget* parent = nullptr);

private:
    QVBoxLayout* m_mainLayout = nullptr;
    InfoCard* m_uptimeCard = nullptr;
    QTimer* m_timer = nullptr;
};

#endif // HARDWAREPAGE_H