#pragma once
#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QElapsedTimer>
#include <QTimer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "../Control/CustomButton.h"
#include "../Control/CustomLineEdit.h"
#include "../Design/AntMessageManager.h"

class RegisterDialog : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget* parent = nullptr);

    void showWithAnimation();   // 显示动画
    void hideWithAnimation();   // 隐藏动画

signals:
    void backToLogin();         // 返回登录

private:
    QPropertyAnimation* posAnim;
    QPropertyAnimation* opacityAnim;
    QWidget* mask = nullptr;
    QPropertyAnimation* maskAnim = nullptr;
    QGraphicsOpacityEffect* maskOpacity = nullptr;
    QParallelAnimationGroup* animGroup = nullptr;
    QElapsedTimer* animTimer = nullptr;
};