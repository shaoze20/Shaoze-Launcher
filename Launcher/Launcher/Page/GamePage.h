#pragma once

#include <QLabel>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath> 
#include <QHBoxLayout>
#include <QStackedWidget>

#include "../Game/TkaPage.h"
#include "../Game/WuwaPage.h"
#include "../Game/YsPage.h"
#include "../Game/PvzPage.h"

#include "../Control/CustomTabWidget.h"
#include "../Control/FeatureCard.h"

class GamePage : public QWidget
{
    Q_OBJECT

public:
    explicit GamePage(QWidget* parent = nullptr);

private:

};