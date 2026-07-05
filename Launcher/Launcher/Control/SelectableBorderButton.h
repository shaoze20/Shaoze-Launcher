#pragma once

#include <QAbstractButton>
#include <QPainter>
#include <QStyleOption>

class SelectableBorderButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit SelectableBorderButton(const QString& text = QString(), QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};