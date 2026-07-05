#include "BannerCardWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QResizeEvent>
#include <QEasingCurve>
#include <QNetworkRequest>
#include <QApplication>

BannerCardWidget::BannerCardWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setMinimumSize(420, 240);

    m_timer = new QTimer(this);
    m_timer->setInterval(5000);

    m_slideAnim = new QPropertyAnimation(this, "slideOffset", this);
    m_slideAnim->setDuration(600);
    m_slideAnim->setEasingCurve(QEasingCurve::InOutCubic);

    m_dotAnim = new QPropertyAnimation(this, "dotOffset", this);
    m_dotAnim->setDuration(600);
    m_dotAnim->setEasingCurve(QEasingCurve::InOutCubic);

    m_net = new QNetworkAccessManager(this);

    m_btnContainer = new QWidget(this);
    m_btnContainer->setAttribute(Qt::WA_TranslucentBackground);
    m_btnContainer->setStyleSheet("background: transparent;");

    m_btnCurrent = new CustomButton("前往官网", m_btnContainer);
    m_btnCurrent->setFixedSize(120, 33);
    m_btnCurrent->show();

    m_btnNext = new CustomButton("前往官网", m_btnContainer);
    m_btnNext->setFixedSize(120, 33);
    m_btnNext->hide();

    m_btnContainer->raise();

    connect(m_timer, &QTimer::timeout, this, &BannerCardWidget::nextSlide);

    connect(m_btnCurrent, &QPushButton::clicked, this, [this]() {
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
            const QString url = m_items[m_currentIndex].officialUrl;
            if (!url.isEmpty()) {
                emit officialClicked(url);
                QDesktopServices::openUrl(QUrl(url));
            }
        }
        });

    connect(m_btnNext, &QPushButton::clicked, this, [this]() {
        if (m_nextIndex >= 0 && m_nextIndex < m_items.size()) {
            const QString url = m_items[m_nextIndex].officialUrl;
            if (!url.isEmpty()) {
                emit officialClicked(url);
                QDesktopServices::openUrl(QUrl(url));
            }
        }
        });

    connect(m_slideAnim, &QPropertyAnimation::finished, this, [this]() {
        if (m_nextIndex >= 0 && m_nextIndex < m_items.size()) {
            m_currentIndex = m_nextIndex;
        }
        m_nextIndex = -1;
        m_slideOffset = 0.0;

        m_dotFromIndex = m_currentIndex;
        m_dotToIndex = m_currentIndex;
        m_dotOffset = 0.0;

        m_btnNext->hide();
        m_btnCurrent->show();

        update();
        });

    connect(m_net, &QNetworkAccessManager::finished,
        this, &BannerCardWidget::onImageDownloaded);
}

void BannerCardWidget::setItems(const QVector<BannerItem>& items)
{
    m_items = items;
    m_currentIndex = 0;
    m_nextIndex = -1;
    m_slideOffset = 0.0;
    m_pixmapCache.clear();

    for (int i = 0; i < m_items.size(); ++i) {
        ensureImageLoaded(i);
    }

    if (m_items.size() > 1) {
        m_timer->start();
    }
    else {
        m_timer->stop();
    }

    update();
}

void BannerCardWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }

    if (index == m_currentIndex) {
        return;
    }

    if (m_slideAnim->state() == QAbstractAnimation::Running) {
        m_slideAnim->stop();
        if (m_nextIndex >= 0 && m_nextIndex < m_items.size()) {
            m_currentIndex = m_nextIndex;
        }
        m_nextIndex = -1;
        m_slideOffset = 0.0;
    }

    m_currentIndex = index;
    update();
}

void BannerCardWidget::setSlideOffset(qreal offset)
{
    m_slideOffset = offset;
    update();
}

void BannerCardWidget::ensureImageLoaded(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }

    const QString& url = m_items[index].imageUrl;
    if (url.isEmpty() || m_pixmapCache.contains(url)) {
        return;
    }

    QNetworkRequest request(QUrl::fromUserInput(url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
    QNetworkReply* reply = m_net->get(request);
    m_replyUrlMap.insert(reply, url);
}

void BannerCardWidget::onImageDownloaded(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }

    const QString url = m_replyUrlMap.take(reply);

    if (reply->error() == QNetworkReply::NoError && !url.isEmpty()) {
        QImage img;
        if (img.loadFromData(reply->readAll())) {
            // 控制缓存图尺寸，避免过大占用内存
            QPixmap pix = QPixmap::fromImage(
                img.scaled(1600, 900, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
            m_pixmapCache.insert(url, pix);
            update();
        }
    }

    reply->deleteLater();
}

void BannerCardWidget::nextSlide()
{
    if (m_items.size() <= 1 || m_slideAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    const int nextIndex = (m_currentIndex + 1) % m_items.size();
    startSlideAnimation(nextIndex);
}

void BannerCardWidget::startSlideAnimation(int nextIndex)
{
    if (nextIndex < 0 || nextIndex >= m_items.size() || nextIndex == m_currentIndex) {
        return;
    }

    m_nextIndex = nextIndex;

    m_dotFromIndex = m_currentIndex;
    m_dotToIndex = nextIndex;

    m_slideAnim->stop();
    m_slideAnim->setStartValue(0.0);
    m_slideAnim->setEndValue(width() - 16);
    m_slideAnim->start();

    m_dotAnim->stop();
    m_dotAnim->setStartValue(0.0);
    m_dotAnim->setEndValue(1.0);
    m_dotAnim->start();
}

QRect BannerCardWidget::officialButtonRect(const QRectF& cardRect) const
{
    const int btnW = 96;
    const int btnH = 32;
    const int leftPadding = 14;
    const int left = int(cardRect.left()) + leftPadding;
    const qreal nameTop = cardRect.top() + cardRect.height() * 0.45;
    const qreal nameHeight = 36.0;
    const qreal descTop = nameTop + nameHeight + 6.0;
    const qreal descHeight = 20.0;
    const int top = int(descTop + descHeight);

    return QRect(left, top, btnW, btnH);
}

QVector<QRect> BannerCardWidget::dotRects(const QRectF& cardRect) const
{
    QVector<QRect> rects;
    if (m_items.isEmpty()) {
        return rects;
    }

    const int dotCount = m_items.size();
    const int dotSize = 8;
    const int spacing = 10;
    const int y = int(cardRect.bottom()) - 24;

    int totalWidth = dotCount * dotSize + (dotCount - 1) * spacing;
    int x = int(cardRect.center().x()) - totalWidth / 2;

    for (int i = 0; i < dotCount; ++i) {
        rects.push_back(QRect(x, y, dotSize, dotSize));
        x += dotSize + spacing;
    }

    return rects;
}

void BannerCardWidget::drawSlideContent(QPainter& painter, const QRectF& rect, int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= m_items.size()) {
        return;
    }

    const BannerItem& item = m_items[itemIndex];

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QPixmap bg = m_pixmapCache.value(item.imageUrl);
    if (!bg.isNull()) {
        painter.drawPixmap(rect.toRect(), bg);
    }
    else {
        painter.fillRect(rect, QColor(220, 220, 220));
        painter.setPen(QColor(130, 130, 130));
        QFont f = painter.font();
        f.setPointSize(12);
        painter.setFont(f);
        painter.drawText(rect, Qt::AlignCenter, QStringLiteral("加载中..."));
    }

    QLinearGradient mask(rect.topLeft(), rect.bottomLeft());
    mask.setColorAt(0.0, QColor(0, 0, 0, 40));
    mask.setColorAt(0.55, QColor(0, 0, 0, 90));
    mask.setColorAt(1.0, QColor(0, 0, 0, 160));
    painter.fillRect(rect, mask);

    const int leftPadding = 24;

    QRectF nameRect(
        rect.left() + leftPadding,
        rect.top() + rect.height() * 0.45,
        rect.width() - leftPadding * 2,
        30
    );

    QFont nameFont = painter.font();
    nameFont.setPointSize(18);
    nameFont.setBold(true);
    painter.setFont(nameFont);
    painter.setPen(Qt::white);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, item.name);

    QRectF descRect(
        rect.left() + leftPadding,
        nameRect.bottom() + 5,
        rect.width() - leftPadding * 2,
        35
    );

    QFont descFont = painter.font();
    descFont.setPointSize(11);
    descFont.setBold(false);
    painter.setFont(descFont);
    painter.setPen(QColor(235, 235, 235));
    painter.drawText(descRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, item.description);

    painter.restore();
}

void BannerCardWidget::drawOverlay(QPainter& painter, const QRectF& cardRect)
{
    if (m_items.isEmpty()) {
        return;
    }

    const int dotCount = m_items.size();
    const qreal normalSize = 8.0;
    const qreal activeSize = 12.0;
    const int spacing = 12;
    const int y = int(cardRect.bottom()) - 26;

    // 固定中心点布局，不跟着页面滑
    qreal totalWidth = dotCount * normalSize + (dotCount - 1) * spacing;
    qreal startX = cardRect.center().x() - totalWidth / 2.0;

    QVector<QPointF> centers;
    qreal x = startX;
    for (int i = 0; i < dotCount; ++i) {
        centers.push_back(QPointF(x + normalSize / 2.0, y + normalSize / 2.0));
        x += normalSize + spacing;
    }

    int fromIndex = qBound(0, m_dotFromIndex, dotCount - 1);
    int toIndex = qBound(0, m_dotToIndex, dotCount - 1);

    for (int i = 0; i < dotCount; ++i) {
        qreal t = 0.0;

        if (m_dotAnim->state() == QAbstractAnimation::Running) {
            if (i == fromIndex) {
                t = 1.0 - m_dotOffset;
            }
            else if (i == toIndex) {
                t = m_dotOffset;
            }
        }
        else {
            t = (i == m_currentIndex) ? 1.0 : 0.0;
        }

        qreal size = normalSize + (activeSize - normalSize) * t;
        QColor color = (t > 0.01) ? QColor(255, 255, 255) : QColor(255, 255, 255, 130);

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);

        QRectF dotRect(
            centers[i].x() - size / 2.0,
            centers[i].y() - size / 2.0,
            size,
            size
        );
        painter.drawEllipse(dotRect);
    }
}

void BannerCardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (m_items.isEmpty()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF cardRect = rect().adjusted(8, 8, -8, -8);
    const qreal radius = 8.0;

    QPainterPath cardPath;
    cardPath.addRoundedRect(cardRect, radius, radius);

    // 卡片底色
    painter.fillPath(cardPath, QColor(245, 245, 245));

    // 圆角
    painter.save();
    painter.setClipPath(cardPath);

    if (m_nextIndex >= 0 && m_nextIndex < m_items.size() && m_slideOffset > 0.0) {
        QRectF currentRect = cardRect.translated(-m_slideOffset, 0);
        QRectF nextRect = cardRect.translated(cardRect.width() - m_slideOffset, 0);

        drawSlideContent(painter, currentRect, m_currentIndex);
        drawSlideContent(painter, nextRect, m_nextIndex);
    }
    else {
        drawSlideContent(painter, cardRect, m_currentIndex);
    }

    painter.restore();

    // 再画固定边框
    QPen borderPen(QColor(190, 190, 190));
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(cardPath);

    // 最后画顶层 dots
    drawOverlay(painter, cardRect);

    if (m_nextIndex >= 0 && m_nextIndex < m_items.size() && m_slideOffset > 0.0) {
        QRectF currentRect = cardRect.translated(-m_slideOffset, 0);
        QRectF nextRect = cardRect.translated(cardRect.width() - m_slideOffset, 0);

        QRect currentBtnRect = officialButtonRect(currentRect);
        QRect nextBtnRect = officialButtonRect(nextRect);

        m_btnCurrent->setGeometry(currentBtnRect);
        m_btnCurrent->show();
        m_btnCurrent->raise();

        m_btnNext->setGeometry(nextBtnRect);
        m_btnNext->show();
        m_btnNext->raise();
    }
    else {
        QRect btnRect = officialButtonRect(cardRect);

        m_btnCurrent->setGeometry(btnRect);
        m_btnCurrent->show();
        m_btnCurrent->raise();

        m_btnNext->hide();
    }

    QRect containerRect = cardRect.toRect();
    m_btnContainer->setGeometry(containerRect);

    // 给按钮容器加圆角遮罩
    QPainterPath path;
    path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(containerRect.size())), 10.0, 10.0);
    QRegion region(path.toFillPolygon().toPolygon());
    m_btnContainer->setMask(region);

    m_btnContainer->raise();
}

void BannerCardWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_items.isEmpty()) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QRectF cardRect = rect().adjusted(8, 8, -8, -8);

    // 点击 dots
    const int dotCount = m_items.size();
    if (dotCount > 0) {
        const qreal normalSize = 8.0;
        const int spacing = 12;
        const int y = int(cardRect.bottom()) - 26;

        qreal totalWidth = dotCount * normalSize + (dotCount - 1) * spacing;
        qreal startX = cardRect.center().x() - totalWidth / 2.0;

        qreal x = startX;
        for (int i = 0; i < dotCount; ++i) {
            QRectF hitRect(x - 6, y - 6, normalSize + 12, normalSize + 12);
            if (hitRect.contains(event->pos())) {
                m_timer->start(5000);

                if (i != m_currentIndex && m_slideAnim->state() != QAbstractAnimation::Running) {
                    startSlideAnimation(i);
                }
                return;
            }

            x += normalSize + spacing;
        }
    }

    // 画普通点
    const QVector<QRect> dots = dotRects(cardRect);
    for (int i = 0; i < dots.size(); ++i) {
        QRect hitRect = dots[i].adjusted(-6, -6, 6, 6);
        if (hitRect.contains(event->pos())) {
            m_timer->start(5000);

            if (i != m_currentIndex && m_slideAnim->state() != QAbstractAnimation::Running) {
                startSlideAnimation(i);
            }
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void BannerCardWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void BannerCardWidget::setDotOffset(qreal offset)
{
    m_dotOffset = offset;
    update();
}