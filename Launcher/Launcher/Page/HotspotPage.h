#pragma once

#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QScrollArea>

#include "../Control/CustomTabWidget.h"
#include "../Control/HotSiteCard.h"
#include "../Style/StyleSheet.h"
#include "../Design/DesignSystem.h"
#include "../Control/BannerCardWidget.h"
#include "../Control/SmoothScrollBar.h"

class HotspotPage : public QWidget
{
    Q_OBJECT

public:
    explicit HotspotPage(QWidget* parent = nullptr);

protected:

};