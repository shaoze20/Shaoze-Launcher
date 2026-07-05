#pragma once
#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QStyleOption>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>

#include "../Gui/MainWindow.h"
#include "../Gui/LoginWindow.h"
#include "../Control/CustomTabWidget.h"
#include "../Control/SettingCard.h"
#include "../Globals/Globals.h"
#include "../Control/CustomButton.h"
#include "../Control/SelectableBorderButton.h"
#include "../Control/LinkLabel.h"

extern SettingCard* m_closeMinimizeCard;
extern SettingCard* m_gameMinimizeCard;
extern SettingCard* m_glassCard;

class SettingPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingPage(QWidget* parent = nullptr);

private slots:
    void onLogoutClicked();

protected:

private:


};