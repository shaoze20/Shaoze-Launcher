#include "HotSiteCard.h"

HotSiteCard::HotSiteCard(const QString& name,
    const QString& description,
    const QString& heatText,
    const QString& url,
    const QString& iconSource,
    QWidget* parent)
    : QWidget(parent),
    m_siteName(name),
    m_description(description),
    m_heatText(heatText),
    m_url(url)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFixedHeight(100);

    setIcon(iconSource);
}

void HotSiteCard::setSiteName(const QString& name)
{
    m_siteName = name;
    update();
}

void HotSiteCard::setDescription(const QString& description)
{
    m_description = description;
    update();
}

void HotSiteCard::setHeatText(const QString& heatText)
{
    m_heatText = heatText;
    update();
}

void HotSiteCard::setIcon(const QString& iconSource)
{
    if (iconSource.trimmed().isEmpty()) {
        m_icon = QPixmap();
        update();
        return;
    }

    const QString src = iconSource.trimmed();

    // 网络图片
    if (src.startsWith("http://", Qt::CaseInsensitive) ||
        src.startsWith("https://", Qt::CaseInsensitive)) {

        if (!m_netManager) {
            m_netManager = new QNetworkAccessManager(this);
        }

        QNetworkRequest request{ QUrl(src) };
        request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

        QNetworkReply* reply = m_netManager->get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "图片加载失败:" << reply->errorString();
                return;
            }

            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                m_icon = pix;
                update();
            }
            });

        return;
    }

    // 本地/资源图片
    QPixmap pix(src);
    m_icon = pix;
    update();
}

QString HotSiteCard::siteName() const
{
    return m_siteName;
}

QString HotSiteCard::description() const
{
    return m_description;
}

QString HotSiteCard::heatText() const
{
    return m_heatText;
}

QPixmap HotSiteCard::icon() const
{
    return m_icon;
}

QSize HotSiteCard::sizeHint() const
{
    return QSize(420, 120);
}

void HotSiteCard::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);
    m_hovered = true;
    update();
}

void HotSiteCard::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    m_hovered = false;
    m_pressed = false;
    update();
}

void HotSiteCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void HotSiteCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(Qt::NoPen);

    QRectF cardRect = rect().adjusted(8, 6, -8, -6);

    drawShadow(painter, cardRect);
    drawBackground(painter, cardRect);
    drawIcon(painter, cardRect);
    drawTexts(painter, cardRect);
    drawHeatTag(painter, cardRect);
}

void HotSiteCard::drawShadow(QPainter& painter, const QRectF& rect)
{
    QColor c1(0, 0, 0, 18);
    QColor c2(0, 0, 0, 10);
    QColor c3(0, 0, 0, 6);

    painter.setPen(Qt::NoPen);

    painter.setBrush(c3);
    painter.drawRoundedRect(rect.adjusted(0, 4, 0, 4), 14, 14);

    painter.setBrush(c2);
    painter.drawRoundedRect(rect.adjusted(0, 2, 0, 2), 14, 14);

    painter.setBrush(c1);
    painter.drawRoundedRect(rect.adjusted(0, 1, 0, 1), 14, 14);
}

void HotSiteCard::drawBackground(QPainter& painter, const QRectF& rect)
{
    QColor bgColor = Qt::white;
    QColor borderColor(230, 232, 236);

    if (m_hovered) {
        bgColor = QColor(252, 253, 255);
        borderColor = QColor(210, 220, 235);
    }

    if (m_pressed) {
        bgColor = QColor(245, 247, 250);
    }

    painter.setBrush(bgColor);
    painter.setPen(QPen(borderColor, 1));
    painter.drawRoundedRect(rect, 14, 14);
}

void HotSiteCard::drawIcon(QPainter& painter, const QRectF& rect)
{
    const qreal iconSize = 65;
    const qreal radius = 16.0;

    QRectF iconRect(
        rect.left() + 18,
        rect.center().y() - iconSize / 2.0,
        iconSize,
        iconSize
    );

    // 先画占位背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 247, 250));
    painter.drawRoundedRect(iconRect, radius, radius);

    if (!m_icon.isNull()) {
        QPainterPath clipPath;
        clipPath.addRoundedRect(iconRect, radius, radius);

        painter.save();
        painter.setClipPath(clipPath);

        QPixmap scaled = m_icon.scaled(
            iconRect.size().toSize(),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation
        );

        QPointF drawPos(
            iconRect.center().x() - scaled.width() / 2.0,
            iconRect.center().y() - scaled.height() / 2.0
        );

        painter.drawPixmap(drawPos, scaled);
        painter.restore();
    }
    else {
        QFont f = painter.font();
        f.setPointSize(20);
        f.setBold(true);
        painter.setFont(f);
        painter.setPen(QColor(120, 130, 145));

        QString text = m_siteName.isEmpty() ? "?" : m_siteName.left(1).toUpper();
        painter.drawText(iconRect, Qt::AlignCenter, text);
    }
}

void HotSiteCard::drawTexts(QPainter& painter, const QRectF& rect)
{
    const qreal left = rect.left() + 102;
    const qreal right = rect.right() - 110;
    const qreal width = right - left;
    const qreal centerY = rect.center().y();

    QRectF titleRect(left, centerY - 35, width, 40);
    QRectF descRect(left, centerY + 5, width, 24);

    QFont titleFont = painter.font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(28, 30, 33));

    QFontMetrics titleFm(titleFont);
    QString elidedTitle = titleFm.elidedText(m_siteName, Qt::ElideRight, int(titleRect.width()));
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, elidedTitle);

    QFont descFont = painter.font();
    descFont.setPointSize(11);
    descFont.setBold(false);
    painter.setFont(descFont);
    painter.setPen(QColor(110, 118, 130));

    QFontMetrics descFm(descFont);
    QString elidedDesc = descFm.elidedText(m_description, Qt::ElideRight, int(descRect.width()));
    painter.drawText(descRect, Qt::AlignLeft | Qt::AlignVCenter, elidedDesc);
}

void HotSiteCard::drawHeatTag(QPainter& painter, const QRectF& rect)
{
    if (m_heatText.trimmed().isEmpty()) {
        return;
    }

    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(m_heatText);
    int tagWidth = qMax(72, textWidth + 22);
    int tagHeight = 30;

    QRectF tagRect(
        rect.right() - tagWidth - 18,
        rect.center().y() - tagHeight / 2.0,
        tagWidth,
        tagHeight
    );

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 244, 230));
    painter.drawRoundedRect(tagRect, 14, 14);

    painter.setPen(QColor(230, 135, 40));
    painter.drawText(tagRect, Qt::AlignCenter, m_heatText);
}

void HotSiteCard::setUrl(const QString& url)
{
    m_url = url;
}

QString HotSiteCard::url() const
{
    return m_url;
}

void HotSiteCard::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_url.isEmpty()) {

        QUrl url(m_url);

        if (url.scheme().isEmpty()) {
            url.setScheme("https");
        }

        QDesktopServices::openUrl(url);
    }

    QWidget::mouseDoubleClickEvent(event);
}