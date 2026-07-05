#include "CustomTabWidget.h"

class SlideOverlay : public QWidget
{
public:
    explicit SlideOverlay(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAutoFillBackground(false);
        hide();
    }

    QPixmap oldPixmap;
    QPixmap newPixmap;
    qreal progress = 0.0;
    bool slideToLeft = true;

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.fillRect(rect(), Qt::white);

        if (oldPixmap.isNull() || newPixmap.isNull()) {
            return;
        }

        const int w = width();
        const int h = height();
        const int offset = static_cast<int>(progress * w);

        QRect oldRect, newRect;
        if (slideToLeft) {
            oldRect = QRect(-offset, 0, w, h);
            newRect = QRect(w - offset, 0, w, h);
        }
        else {
            oldRect = QRect(offset, 0, w, h);
            newRect = QRect(-w + offset, 0, w, h);
        }

        p.drawPixmap(oldRect, oldPixmap);
        p.drawPixmap(newRect, newPixmap);
    }
};

class TabBarBackground : public QWidget
{
public:
    explicit TabBarBackground(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(false);
    }

    qreal indicatorX = 0.0;
    qreal indicatorW = 0.0;

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        // 关键：不要用0.5，用1像素内缩
        QRectF bgRect = rect().adjusted(1, 1, -1, -1);

        // 灰色背景 + 边框
        p.setPen(QPen(QColor(210, 210, 210), 1));
        p.setBrush(QColor(235, 235, 235));
        p.drawRoundedRect(bgRect, 8, 8);

        // 白色滑块
        QRectF r(indicatorX, 4.0, indicatorW, height() - 8.0);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawRoundedRect(r, 6, 6);
    }
};

CustomTabWidget::CustomTabWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("CustomTabWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    // 顶部 tab 区域
    m_tabBarWidget = new TabBarBackground(this);
    m_tabBarWidget->setFixedHeight(36);
    m_tabBarWidget->setObjectName("CustomTabBar");
    m_tabBarWidget->setStyleSheet(R"(
        QWidget#CustomTabBar {
            background: transparent;
            border: none;
        }
    )");

    m_tabBarLayout = new QHBoxLayout(m_tabBarWidget);
    m_tabBarLayout->setContentsMargins(15, 0, 15, 0);
    m_tabBarLayout->setSpacing(28);
    m_tabBarLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 下方页面区域
    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->setObjectName("CustomTabStack");
    m_stackWidget->setStyleSheet(R"(
        QStackedWidget#CustomTabStack {
            background: white;
            border: none;
        }
    )");

    mainLayout->addWidget(m_tabBarWidget);
    mainLayout->addWidget(m_stackWidget);

    // 动画
    int interval = 1000 / screen()->refreshRate();
    m_indicatorTimer = new QTimer(this);
    m_indicatorTimer->setTimerType(Qt::PreciseTimer);
    m_indicatorTimer->setInterval(interval);

    m_animOverlay = new SlideOverlay(m_stackWidget);
    m_animOverlay->setGeometry(m_stackWidget->rect());
    m_animOverlay->raise();

    m_frameTimer = new QTimer(this);
    m_frameTimer->setTimerType(Qt::PreciseTimer);
    m_frameTimer->setInterval(interval);

    connect(m_frameTimer, &QTimer::timeout, this, [this]() {
        if (!m_isAnimating) {
            return;
        }

        const qint64 elapsed = m_elapsedTimer.elapsed();
        qreal t = qreal(elapsed) / qreal(m_animDuration);
        if (t > 1.0) t = 1.0;

        // OutCubic
        const qreal k = 1.0 - qPow(1.0 - t, 3.0);
        m_animProgress = k;

        SlideOverlay* overlay = static_cast<SlideOverlay*>(m_animOverlay);
        overlay->progress = m_animProgress;
        overlay->update();

        if (t >= 1.0) {
            finishSlideAnimation();
        }
        });

    connect(m_indicatorTimer, &QTimer::timeout, this, [this]() {

        if (!m_indicatorAnimating) return;

        qreal t = qreal(m_indicatorElapsed.elapsed()) / m_indicatorDuration;
        if (t > 1.0) t = 1.0;

        qreal k = 1.0 - qPow(1.0 - t, 3.0);

        m_indicatorX = m_indicatorStartX + (m_indicatorTargetX - m_indicatorStartX) * k;
        m_indicatorW = m_indicatorStartW + (m_indicatorTargetW - m_indicatorStartW) * k;

        auto bg = static_cast<TabBarBackground*>(m_tabBarWidget);
        bg->indicatorX = m_indicatorX;
        bg->indicatorW = m_indicatorW;
        bg->update();

        if (t >= 1.0) {
            m_indicatorAnimating = false;
            m_indicatorTimer->stop();
        }
        });
}

void CustomTabWidget::addTab(const QString& title, QWidget* page)
{
    QPushButton* btn = new QPushButton(title, m_tabBarWidget);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    btn->setAutoExclusive(true);
    btn->setFixedHeight(28);
    btn->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #555555;
            text-align: center;
            font-size: 14px;
            font-weight: 500;
            padding: 6px 14px;
            border-radius: 8px;
        }
        QPushButton:hover {
            background: transparent;
            color: #555555;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton:checked {
            background: transparent;
            color: #1E90FF;
            font-size: 14px;
            font-weight: 500;
        }
    )");

    const int index = m_tabButtons.size();
    connect(btn, &QPushButton::clicked, this, [=]() {
        setCurrentIndex(index);
        });

    m_tabButtons.push_back(btn);
    m_tabBarLayout->addWidget(btn);
    m_stackWidget->addWidget(page);
    btn->raise();

    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        m_stackWidget->setCurrentIndex(0);
        updateTabStyles();
        updateIndicator(false);
    }
}

void CustomTabWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_tabButtons.size()) {
        updateTabStyles();
        return;
    }

    if (m_isAnimating) {
        updateTabStyles();
        return;
    }

    if (m_currentIndex == -1) {
        m_currentIndex = index;
        m_stackWidget->setCurrentIndex(index);
        updateTabStyles();
        updateIndicator(false);
        return;
    }

    if (m_currentIndex == index) {
        updateTabStyles();
        return;
    }

    int oldIndex = m_currentIndex;
    m_currentIndex = index;

    updateTabStyles();
    updateIndicator(true);

    m_animFromIndex = oldIndex;
    m_animToIndex = index;
    startSlideAnimation(index);
}

int CustomTabWidget::currentIndex() const
{
    return m_currentIndex;
}

void CustomTabWidget::updateTabStyles()
{
    for (int i = 0; i < m_tabButtons.size(); ++i) {
        m_tabButtons[i]->setChecked(i == m_currentIndex);
    }
}

void CustomTabWidget::updateIndicator(bool animate)
{
    if (m_currentIndex < 0 || m_currentIndex >= m_tabButtons.size()) return;

    QPushButton* btn = m_tabButtons[m_currentIndex];
    if (!btn) return;

    QRect geo = btn->geometry();

    const int expand = 10;

    qreal targetX = geo.x() - expand;
    qreal targetW = geo.width() + expand * 2;

    if (!animate) {
        m_indicatorX = targetX;
        m_indicatorW = targetW;

        auto bg = static_cast<TabBarBackground*>(m_tabBarWidget);
        bg->indicatorX = m_indicatorX;
        bg->indicatorW = m_indicatorW;
        bg->update();
        return;
    }

    m_indicatorStartX = m_indicatorX;
    m_indicatorStartW = m_indicatorW;

    m_indicatorTargetX = targetX;
    m_indicatorTargetW = targetW;

    m_indicatorAnimating = true;
    m_indicatorElapsed.restart();
    m_indicatorTimer->start();
}

void CustomTabWidget::switchPageWithSlide(int newIndex)
{
    QWidget* nextWidget = m_stackWidget->widget(newIndex);
    QWidget* currentWidget = m_stackWidget->currentWidget();

    if (!nextWidget || !currentWidget || nextWidget == currentWidget) {
        m_stackWidget->setCurrentIndex(newIndex);
        return;
    }

    m_isAnimating = true;

    const QRect area = m_stackWidget->rect();
    const int w = area.width();
    const int h = area.height();

    const int oldIndex = m_stackWidget->currentIndex();
    const bool moveLeft = (newIndex > oldIndex);

    currentWidget->setParent(m_stackWidget);
    nextWidget->setParent(m_stackWidget);

    currentWidget->show();
    nextWidget->show();
    currentWidget->raise();

    currentWidget->setGeometry(0, 0, w, h);
    nextWidget->setGeometry(moveLeft ? w : -w, 0, w, h);
    nextWidget->raise();

    QPropertyAnimation* animCurrent = new QPropertyAnimation(currentWidget, "pos");
    QPropertyAnimation* animNext = new QPropertyAnimation(nextWidget, "pos");
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

    animCurrent->setDuration(260);
    animNext->setDuration(260);

    animCurrent->setEasingCurve(QEasingCurve::OutCubic);
    animNext->setEasingCurve(QEasingCurve::OutCubic);

    animCurrent->setStartValue(QPoint(0, 0));
    animCurrent->setEndValue(QPoint(moveLeft ? -w : w, 0));

    animNext->setStartValue(QPoint(moveLeft ? w : -w, 0));
    animNext->setEndValue(QPoint(0, 0));

    group->addAnimation(animCurrent);
    group->addAnimation(animNext);

    connect(group, &QParallelAnimationGroup::finished, this, [=]() {
        m_stackWidget->setCurrentIndex(newIndex);

        // 恢复最终布局
        currentWidget->hide();
        currentWidget->move(0, 0);
        nextWidget->move(0, 0);
        nextWidget->setGeometry(0, 0, w, h);

        m_isAnimating = false;
        group->deleteLater();
        });

    group->start();
}

void CustomTabWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateIndicator(false);

    if (m_animOverlay) {
        m_animOverlay->setGeometry(m_stackWidget->rect());
    }

    if (QWidget* current = m_stackWidget->currentWidget()) {
        current->setGeometry(m_stackWidget->rect());
    }
}

void CustomTabWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void CustomTabWidget::startSlideAnimation(int newIndex)
{
    QWidget* oldPage = m_stackWidget->widget(m_animFromIndex);
    QWidget* newPage = m_stackWidget->widget(newIndex);

    if (!oldPage || !newPage || oldPage == newPage) {
        m_stackWidget->setCurrentIndex(newIndex);
        return;
    }

    newPage->setGeometry(m_stackWidget->rect());

    const int currentRealIndex = m_stackWidget->currentIndex();
    m_stackWidget->setCurrentIndex(newIndex);
    qApp->processEvents();

    m_newPixmap = newPage->grab();

    m_stackWidget->setCurrentIndex(m_animFromIndex);
    qApp->processEvents();

    m_oldPixmap = oldPage->grab();

    m_slideToLeft = (newIndex > m_animFromIndex);

    SlideOverlay* overlay = static_cast<SlideOverlay*>(m_animOverlay);
    overlay->oldPixmap = m_oldPixmap;
    overlay->newPixmap = m_newPixmap;
    overlay->progress = 0.0;
    overlay->slideToLeft = m_slideToLeft;
    overlay->setGeometry(m_stackWidget->rect());
    overlay->show();
    overlay->raise();

    oldPage->hide();

    m_isAnimating = true;
    m_animProgress = 0.0;
    m_elapsedTimer.restart();
    m_frameTimer->start();
}

void CustomTabWidget::finishSlideAnimation()
{
    m_frameTimer->stop();
    m_isAnimating = false;

    m_stackWidget->setCurrentIndex(m_animToIndex);

    if (QWidget* page = m_stackWidget->currentWidget()) {
        page->show();
        page->setGeometry(m_stackWidget->rect());
    }

    SlideOverlay* overlay = static_cast<SlideOverlay*>(m_animOverlay);
    overlay->hide();
    overlay->oldPixmap = QPixmap();
    overlay->newPixmap = QPixmap();
    overlay->progress = 0.0;

    m_animFromIndex = -1;
    m_animToIndex = -1;
}