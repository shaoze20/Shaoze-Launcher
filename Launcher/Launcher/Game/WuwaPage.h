#pragma once
#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QApplication>
#include <QProcess>

#include "../Page/SettingPage.h"
#include "../Gui/MainWindow.h"
#include "../Design/DesignSystem.h"
#include "../Control/StartButton.h"
#include "../Control/MoreOptionButton.h"
#include "../Control/OutlineButton.h"
#include "../Control/NoticeCard.h"
#include "../Design/AntMessageManager.h"
#include "../Menu/PathDialog.h"
#include "../Menu/InjectDialog.h"
#include "../Control/BackButton.h"

class WuwaPage : public QWidget
{
    Q_OBJECT

public:
    explicit WuwaPage(QWidget* parent = nullptr);

    void showWithFade();
    void hideWithFade();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void GameProgress();

private:
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;
    QPropertyAnimation* m_opacityAnim = nullptr;
    bool m_isFading = false;

    QPixmap m_bgOriginal;

    NoticeCard* m_notice = nullptr;
    StartButton* m_startBtn = nullptr;
    MoreOptionButton* m_pachBtn = nullptr;
    BackButton* backBtn = nullptr;

    QString WuwaPach = "";
    QString WuwaModPach = "";

    bool m_isLaunching = false;//启动状态
    bool m_hasMinimized = false; // 窗口最小化
    bool vidaBtn = true;
    bool modBtn = false;
};