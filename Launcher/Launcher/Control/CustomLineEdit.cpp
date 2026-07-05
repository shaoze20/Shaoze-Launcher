#include "CustomLineEdit.h"
#include <QFocusEvent>

CustomLineEdit::CustomLineEdit(QWidget* parent)
    : QLineEdit(parent),
    m_borderColor(QColor("#dcdcdc"))
{
    setFixedSize(300, 50);

    setAlignment(Qt::AlignCenter);
    setPlaceholderText("请输入或选择游戏安装目录...");

    m_anim = new QPropertyAnimation(this, "borderColor", this);
    m_anim->setDuration(200);

    updateStyle();
}

QColor CustomLineEdit::borderColor() const
{
    return m_borderColor;
}

void CustomLineEdit::setBorderColor(const QColor& color)
{
    m_borderColor = color;
    updateStyle();
}

void CustomLineEdit::updateStyle()
{
    QString style = QString(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid %1;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
            padding: 5px;
        }
    )").arg(m_borderColor.name());

    setStyleSheet(style);
}

void CustomLineEdit::focusInEvent(QFocusEvent* event)
{
    m_anim->stop();
    m_anim->setStartValue(m_borderColor);
    m_anim->setEndValue(QColor(0, 120, 215)); // Win11 蓝
    m_anim->start();

    QLineEdit::focusInEvent(event);
}

void CustomLineEdit::focusOutEvent(QFocusEvent* event)
{
    m_anim->stop();
    m_anim->setStartValue(m_borderColor);
    m_anim->setEndValue(QColor("#dcdcdc"));
    m_anim->start();

    QLineEdit::focusOutEvent(event);
}