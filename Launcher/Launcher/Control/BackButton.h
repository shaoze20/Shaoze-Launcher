#pragma once
#include <QWidget>

class BackButton : public QWidget
{
    Q_OBJECT

public:
    explicit BackButton(QWidget* parent = nullptr);

    void setDiameter(int diameter);
    int diameter() const;

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int m_diameter = 40;
    bool m_hovered = false;
    bool m_pressed = false;
};