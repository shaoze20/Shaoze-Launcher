#pragma once
#include <QWidget>
#include <qlineedit.h>
#include <qboxlayout.h>
#include <qplaintextedit.h>
#include <QGraphicsDropShadowEffect>

class SendBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SendBarWidget(QWidget* parent = nullptr);

    QPlainTextEdit* input() const { return m_input; }

    QHBoxLayout* topLeftLayout() const { return m_topLeftLay; }
    QHBoxLayout* bottomRightLayout() const { return m_bottomRightLay; }

signals:
    void sendRequested(const QString& text);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void adjustInputHeight();

private:
    QPlainTextEdit* m_input = nullptr;

    QHBoxLayout* m_topLeftLay = nullptr;
    QHBoxLayout* m_bottomRightLay = nullptr;

    int m_radius = 12;
    int m_border = 1;
    int m_minEditH = 130;
    int m_maxEditH = 150;
};