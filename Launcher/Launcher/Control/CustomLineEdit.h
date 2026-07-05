#pragma once
#include <QLineEdit>
#include <QPropertyAnimation>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
        Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)

public:
    explicit CustomLineEdit(QWidget* parent = nullptr);

    QColor borderColor() const;
    void setBorderColor(const QColor& color);

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void updateStyle();

private:
    QColor m_borderColor;
    QPropertyAnimation* m_anim;
};