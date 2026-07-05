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

class NoticeDialog : public QWidget
{
    Q_OBJECT

public:
    explicit NoticeDialog(QWidget* parent = nullptr);

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