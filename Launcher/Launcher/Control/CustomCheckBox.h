#pragma once
#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QAbstractButton>
#include <QVariantAnimation>
#include <QPainterPath>

class CustomCheckBox : public QAbstractButton
{
    Q_OBJECT
        Q_PROPERTY(qreal animValue READ animValue WRITE setAnimValue)

public:
    explicit CustomCheckBox(const QString& text, QWidget* parent = nullptr);

    qreal animValue() const { return m_animValue; }
    void setAnimValue(qreal value) { m_animValue = value; update(); }
    void setChecked(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void nextCheckState() override; // 处理点击切换状态

private:
    qreal m_animValue = 0.0; // 0.0 为未选中，1.0 为完全选中
    QVariantAnimation* m_animation;
    const int m_boxSize = 16; // 复选框的正方形大小
};

#endif // CUSTOMCHECKBOX_H