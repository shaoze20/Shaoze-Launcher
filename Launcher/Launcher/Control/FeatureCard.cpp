#include "FeatureCard.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QNetworkRequest>
#include <QUrl>
#include <QFontMetrics>

FeatureCard::FeatureCard(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(360, 220);

    // 给阴影留空间
    setContentsMargins(12, 12, 12, 12);

    static QNetworkAccessManager* s_net = new QNetworkAccessManager(qApp);
    m_net = s_net;

    connect(m_net, &QNetworkAccessManager::finished,
        this, &FeatureCard::onImageDownloaded);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(24);
    m_shadowEffect->setOffset(0, 0);
    m_shadowEffect->setColor(QColor(0, 0, 0, 45));
    setGraphicsEffect(m_shadowEffect);
}

void FeatureCard::setName(const QString& name)
{
    m_name = name;
    update();
}

void FeatureCard::setDescription(const QString& description)
{
    m_description = description;
    update();
}

void FeatureCard::setIcon(const QPixmap& icon)
{
    m_icon = icon;
    updateScaledPixmaps();
    update();
}

void FeatureCard::setImageUrl(const QString& url)
{
    if (m_imageUrl == url && !m_imageOriginal.isNull()) {
        return;
    }

    m_imageUrl = url;
    m_imageOriginal = QPixmap();
    m_imageScaled = QPixmap();
    m_lastImageScaledTarget = QSize();

    QNetworkRequest request;
    request.setUrl(QUrl::fromUserInput(url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    m_net->get(request);
}

void FeatureCard::onImageDownloaded(QNetworkReply* reply)
{
    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    if (reply->url() != QUrl::fromUserInput(m_imageUrl)) {
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();

    QImage image;
    if (image.loadFromData(data)) {
        const int topH = height() * 2 / 3;
        qreal dpr = devicePixelRatioF();
        if (windowHandle() && windowHandle()->screen()) {
            dpr = windowHandle()->screen()->devicePixelRatio();
        }

        const int cacheW = qMax(qRound(width() * dpr), 1200);
        const int cacheH = qMax(qRound(topH * dpr), 800);

        if (image.width() > cacheW || image.height() > cacheH) {
            image = image.scaled(
                cacheW, cacheH,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
        }

        m_imageOriginal = QPixmap::fromImage(image);
        updateScaledPixmaps();
        update();
    }

    reply->deleteLater();
}

void FeatureCard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateScaledPixmaps();
}

void FeatureCard::updateScaledPixmaps()
{
    if (!m_imageOriginal.isNull()) {
        const int topH = height() * 2 / 3;
        const QSize targetSize(width(), topH);

        if (m_lastImageScaledTarget != targetSize) {
            m_imageScaled = m_imageOriginal.scaled(
                targetSize,
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation
            );
            m_lastImageScaledTarget = targetSize;
        }
    }

    if (!m_icon.isNull()) {
        int iconSize = qBound(52, qMin(width(), height()) / 4, 72);
        const QSize iconTarget(iconSize, iconSize);

        if (m_lastIconScaledTarget != iconTarget) {
            m_iconScaled = m_icon.scaled(
                iconTarget,
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation
            );
            m_lastIconScaledTarget = iconTarget;
        }
    }
}

QPainterPath FeatureCard::roundedPath(const QRectF& rect, qreal radius) const
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

void FeatureCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF cardRect = rect().adjusted(1.5, 1.5, -1.5, -1.5);
    const qreal radius = 16.0;

    const int topH = height() * 2 / 3;
    const QRectF topRect(0, 0, width(), topH);
    const QRectF bottomRect(0, topH, width(), height() - topH);

    QColor bgColor(235, 235, 235);
    QColor borderColor = m_hovered ? QColor(40, 120, 255) : QColor(180, 180, 180);
    QColor bottomBg(242, 242, 242);

    if (m_pressed) {
        bgColor = QColor(228, 228, 228);
        bottomBg = QColor(236, 236, 236);
    }


    // 整体卡片底色
    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawPath(roundedPath(cardRect, radius));

    // 顶部图片区域
    QPainterPath topPath;
    topPath.moveTo(radius, 0);
    topPath.lineTo(width() - radius, 0);
    topPath.quadTo(width(), 0, width(), radius);
    topPath.lineTo(width(), topH);
    topPath.lineTo(0, topH);
    topPath.lineTo(0, radius);
    topPath.quadTo(0, 0, radius, 0);

    p.save();
    p.setClipPath(topPath);

    if (!m_imageScaled.isNull()) {
        p.drawPixmap(topRect.toRect(), m_imageScaled);
    }
    else {
        p.fillRect(topRect, QColor(210, 210, 210));
        p.setPen(QColor(130, 130, 130));
        QFont f = font();
        f.setPointSize(12);
        p.setFont(f);
        p.drawText(topRect, Qt::AlignCenter, QStringLiteral("无法加载"));
    }
    p.restore();

    // 底部区域
    QPainterPath bottomPath;
    bottomPath.moveTo(0, topH);
    bottomPath.lineTo(width(), topH);
    bottomPath.lineTo(width(), height() - radius);
    bottomPath.quadTo(width(), height(), width() - radius, height());
    bottomPath.lineTo(radius, height());
    bottomPath.quadTo(0, height(), 0, height() - radius);
    bottomPath.lineTo(0, topH);

    p.save();
    p.setClipPath(bottomPath);
    p.fillRect(bottomRect, bottomBg);
    p.restore();

    // 分隔线
    p.setPen(QPen(QColor(210, 210, 210), 1));
    p.drawLine(QPointF(14, topH), QPointF(width() - 14, topH));

    // 布局参数
    const int marginL = 10;
    const int marginR = 16;
    const int bottomY = topH;
    const int bottomH = height() - topH;

    int iconSize = m_iconScaled.isNull() ? 56 : m_iconScaled.width();
    int iconX = marginL;
    int iconY = bottomY + (bottomH - iconSize) / 2;

    // 图标圆角
    QRectF iconRect(iconX, iconY, iconSize, iconSize);
    QPainterPath iconClip;
    iconClip.addRoundedRect(iconRect, 12, 12);

    p.save();
    p.setClipPath(iconClip);

    if (!m_iconScaled.isNull()) {
        p.drawPixmap(iconRect.toRect(), m_iconScaled);
    }
    else {
        p.fillRect(iconRect, QColor(160, 160, 160));
    }

    p.restore();

    // 右侧文字区域
    const int textX = iconX + iconSize + 16;
    const int textW = width() - textX - marginR;

    // 名字在上
    QRectF nameRect(textX, iconY + 1, textW, 24);
    p.setPen(QColor(45, 45, 45));
    QFont nameFont = font();
    nameFont.setPointSize(15);
    nameFont.setBold(true);
    p.setFont(nameFont);
    p.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, m_name);

    // 描述在下
    QRectF descRect(textX, bottomY + 35, textW, bottomH - 40);
    p.setPen(QColor(90, 90, 90));
    QFont descFont = font();
    descFont.setPointSize(12);
    descFont.setBold(false);
    p.setFont(descFont);
    p.drawText(descRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, m_description);
}

void FeatureCard::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);
    m_hovered = true;
    updateShadowEffect();
    update();
}

void FeatureCard::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    m_hovered = false;
    m_pressed = false;
    updateShadowEffect();
    update();
}

void FeatureCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        updateShadowEffect();
        update();
    }
    QWidget::mousePressEvent(event);
}

void FeatureCard::mouseReleaseEvent(QMouseEvent* event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    updateShadowEffect();
    update();

    if (wasPressed && rect().contains(event->pos()) && event->button() == Qt::LeftButton) {
        emit clicked();
    }

    QWidget::mouseReleaseEvent(event);
}

void FeatureCard::updateShadowEffect()
{
    if (!m_shadowEffect)
        return;

    if (m_pressed) {
        m_shadowEffect->setBlurRadius(10);
        m_shadowEffect->setOffset(0, 0);
        m_shadowEffect->setColor(QColor(0, 0, 0, 70));
    }
    else if (m_hovered) {
        m_shadowEffect->setBlurRadius(22);
        m_shadowEffect->setOffset(0, 0);
        m_shadowEffect->setColor(QColor(30, 144, 255, 255));
    }
    else {
        m_shadowEffect->setBlurRadius(16);
        m_shadowEffect->setOffset(0, 0);
        m_shadowEffect->setColor(QColor(0, 0, 0, 90));
    }
}