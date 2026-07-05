#pragma once

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QFrame>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QResizeEvent>
#include <QEasingCurve>
#include <QPainter>
#include <QApplication>
#include <QTimer>

class CustomTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomTabWidget(QWidget* parent = nullptr);

    void addTab(const QString& title, QWidget* page);
    void setCurrentIndex(int index);
    int currentIndex() const;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updateTabStyles();
    void updateIndicator(bool animate);
    void switchPageWithSlide(int newIndex);
    void startSlideAnimation(int newIndex);
    void finishSlideAnimation();

private:
    QWidget* m_tabBarWidget = nullptr;
    QHBoxLayout* m_tabBarLayout = nullptr;
    QStackedWidget* m_stackWidget = nullptr;

    QVector<QPushButton*> m_tabButtons;
    int m_currentIndex = -1;

    QPropertyAnimation* m_anim = nullptr;   // 指示条动画

    // 高刷切页动画
    QWidget* m_animOverlay = nullptr;
    QTimer* m_frameTimer = nullptr;
    QElapsedTimer m_elapsedTimer;

    QPixmap m_oldPixmap;
    QPixmap m_newPixmap;

    int m_animFromIndex = -1;
    int m_animToIndex = -1;
    int m_animDuration = 260;     //    动画速度
    qreal m_animProgress = 0.0;
    bool m_isAnimating = false;
    bool m_slideToLeft = true;

    // 指示条动画
    QTimer* m_indicatorTimer = nullptr;
    QElapsedTimer m_indicatorElapsed;

    qreal m_indicatorX = 0;
    qreal m_indicatorW = 0;

    qreal m_indicatorStartX = 0;
    qreal m_indicatorStartW = 0;

    qreal m_indicatorTargetX = 0;
    qreal m_indicatorTargetW = 0;

    bool m_indicatorAnimating = false;
    int m_indicatorDuration = 260;
};