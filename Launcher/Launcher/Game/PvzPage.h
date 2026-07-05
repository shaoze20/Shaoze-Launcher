#pragma once
#include <QWidget>
#include <QStyleOption>
#include <QPainterPath>
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

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

class PvzPage : public QWidget
{
    Q_OBJECT

public:
    explicit PvzPage(QWidget* parent = nullptr);

    void showWithFade();
    void hideWithFade();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void GameProgress();
    void setGameStatus(const QString& status);

private:
    QWidget* m_topPanel = nullptr;
    QWidget* m_bottomPanel = nullptr;
    QLabel* m_statusLabel = nullptr;

    MaterialSpinner* m_spinner = nullptr;

    QGraphicsOpacityEffect* m_opacityEffect = nullptr;
    QPropertyAnimation* m_opacityAnim = nullptr;
    bool m_isFading = false;

    BackButton* backBtn = nullptr;
    CapsuleCheckBox* m_SunlightRestrictionCheck = nullptr;
    CapsuleCheckBox* m_NoCdCheck = nullptr;
    CustomButton* m_ModifysunlightBtn = nullptr;
    CustomLineEdit* m_AmountsunlightEdit = nullptr;
    CustomButton* m_ModifyCoinsBtn = nullptr;
    CustomLineEdit* m_NumbercoinsEdit = nullptr;
};