#pragma once
#include <QPointF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtWidgets/QWidget>
#include <QLabel>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include "ui_Launcher.h"

#include "MainWindow.h"
#include "../Globals/Globals.h"
#include "../Utils/Encrypt.h"
#include "../Design/DesignSystem.h"
#include "../Design/AntMessageManager.h"
#include "../Effect/GlassWidget.h"
#include "../Menu/RegisterDialog.h"
#include "../Menu/NoticeDialog.h"
#include "../Control/CustomLineEdit.h"
#include "../Control/CustomToolButton.h"
#include "../Control/CustomButton.h"
#include "../Control/CustomCheckBox.h"

extern std::string g_version, currentversion;
extern std::vector<std::string> members;
extern bool m_autoLogin;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    NoticeDialog* noticeDialog = nullptr;
    RegisterDialog* registerDialog = nullptr;
    QLabel* imgsLabel;
    QPointF m_dragPosition;
    Ui::LauncherClass ui;
    QImage m_scaledFrame;
    QSize m_lastSize;
};

