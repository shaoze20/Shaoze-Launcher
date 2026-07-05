#pragma once
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QPointF>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QSequentialAnimationGroup>
#include <QSystemTrayIcon>
#include <QMenu>
#include "ui_Launcher.h"

#include "LoginWindow.h"
#include "../Globals/Globals.h"
#include "../Design/DesignSystem.h"
#include "../Control/CustomLineEdit.h"
#include "../Control/CustomToolButton.h"
#include "../Control/CustomGlowButton.h"
#include "../Control/SlideStackedWidget.h"

#include "../Page/HomePage.h"
#include "../Page/HotspotPage.h"
#include "../Page/ChatPage.h"
#include "../Page/SettingPage.h"
#include "../Page/HardwarePage.h"
#include "../Page/GamePage.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void initTrayIcon();

private:
    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_trayMenu = nullptr;

    QPointF m_dragPosition;
    Ui::LauncherClass ui;

    QWidget* m_container = nullptr;
    QWidget* m_leftPanel = nullptr;
    QPoint m_dragPos;
    bool m_dragging = false;
    QWidget* titleBar = nullptr;

    SlideStackedWidget* m_stack;
    QButtonGroup* m_btnGroup;

    QWidget* m_indicator = nullptr;
    bool m_animating = false;
    int m_pendingPageId = -1;
};