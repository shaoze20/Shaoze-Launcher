#include "NoticeCard.h"

NoticeCard::NoticeCard(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

NoticeCard::NoticeCard(const QString& content, QWidget* parent)
    : QWidget(parent), m_content(content)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void NoticeCard::setContent(const QString& content)
{
    if (m_content == content)
        return;

    m_content = content;
    updateGeometry();
    update();
}

QString NoticeCard::content() const
{
    return m_content;
}

QSize NoticeCard::sizeHint() const
{
    return QSize(420, 140);
}

QSize NoticeCard::minimumSizeHint() const
{
    return QSize(180, 80);
}

void NoticeCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF r = rect().adjusted(10, 10, -10, -10); // 给阴影留空间
    const qreal radius = 16.0;

    QPainterPath path;
    path.addRoundedRect(r, radius, radius);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 100)); // 半透明白
    painter.drawPath(path);
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1));
    painter.drawPath(path);

    // 内容区域
    QRectF textRect = r.adjusted(18, 14, -18, -14);

    QFont font = this->font();
    font.setPixelSize(15);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(0, 0, 0));

    painter.drawText(textRect,
        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
        m_content);
}