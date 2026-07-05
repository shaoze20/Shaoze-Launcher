#pragma once
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include "../Menu/PopupPanel.h"

class MoreOptionButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MoreOptionButton(QWidget* parent = nullptr);


    void addMenuButton(QPushButton* btn);// 添加按钮
    void showPopup();
    void hidePopup(bool force = false);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool m_hovered = false;

    PopupPanel* m_popup = nullptr;
    QVBoxLayout* m_layout = nullptr;

    QPropertyAnimation* m_posAnim = nullptr;
    QPropertyAnimation* m_opacityAnim = nullptr;
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;
};