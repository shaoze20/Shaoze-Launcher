#pragma once
#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QPushButton>
#include <QVariantAnimation>
#include <QMouseEvent>

class CustomButton : public QPushButton
{
    Q_OBJECT
        Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit CustomButton(const QString& text, QWidget* parent = nullptr);

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity) { m_opacity = opacity; update(); }

    void setRadius(int r);
    int radius() const;

    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;

    void setTextColor(const QColor& color);
    QColor textColor() const;

    void setFontSize(int size);
    int fontSize() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    qreal m_opacity; // 当前透明度
    QVariantAnimation* m_animation;

    int m_radius = 10;
    int m_fontSize = 12;

    QColor m_bgColor = QColor(0, 120, 215);  // 背景色
    QColor m_textColor = Qt::white;          // 文字颜色
};

#endif // CUSTOMBUTTON_H