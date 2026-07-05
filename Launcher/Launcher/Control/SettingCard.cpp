#include "SettingCard.h"

SettingCard::SettingCard(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SettingCard");
    setFixedHeight(96);
    setAttribute(Qt::WA_StyledBackground, true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_checkBox = new CapsuleCheckBox("", this);

    connect(m_checkBox, &CapsuleCheckBox::toggled, this, &SettingCard::toggled);

    updateCheckBoxGeometry();
}

void SettingCard::setDescription(const QString& description)
{
    if (m_description == description) return;
    m_description = description;
    update();
}

QString SettingCard::description() const
{
    return m_description;
}

void SettingCard::setImage(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
    update();
}

QPixmap SettingCard::image() const
{
    return m_pixmap;
}

void SettingCard::setImageVisible(bool visible)
{
    if (m_imageVisible == visible) return;
    m_imageVisible = visible;
    update();
}

bool SettingCard::isImageVisible() const
{
    return m_imageVisible;
}

void SettingCard::setChecked(bool checked)
{
    if (m_checkBox) {
        m_checkBox->setChecked(checked);
    }
}

bool SettingCard::isChecked() const
{
    return m_checkBox ? m_checkBox->isChecked() : false;
}

void SettingCard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateCheckBoxGeometry();
    update();
}

void SettingCard::updateCheckBoxGeometry()
{
    if (!m_checkBox) return;

    const int h = height();

    // 开关尺寸跟随卡片高度变化
    int switchW = qMax(40, int(h * 0.50));
    int switchH = qMax(20, int(h * 0.35));

    m_checkBox->resize(switchW, switchH);

    // 右边距也跟随高度稍微变化
    const int rightMargin = qMax(16, int(h * 0.25));
    const int x = width() - rightMargin - m_checkBox->width();
    const int y = (height() - m_checkBox->height()) / 2;
    m_checkBox->move(x, y);
}

void SettingCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF cardRect = rect().adjusted(1, 1, -1, -1);

    const int h = height();

    // 各项尺寸都跟随高度变化
    const int radius = qMax(10, int(h * 0.145));
    const int leftMargin = qMax(14, int(h * 0.187));
    const int spacing = qMax(10, int(h * 0.145));
    const int imageSize = qMax(36, int(h * 0.54));
    const int rightReserved = m_checkBox->width() + qMax(24, int(h * 0.45));

    // 背景
    QColor bgColor(248, 251, 255);
    QColor borderColor(220, 232, 245);

    painter.setPen(QPen(borderColor, 1));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(cardRect, radius, radius);

    int textLeft = leftMargin;

    // 图片区域
    if (m_imageVisible) {
        QRect imageRect(leftMargin, (height() - imageSize) / 2, imageSize, imageSize);

        if (!m_pixmap.isNull()) {
            qreal dpr = devicePixelRatioF();
            if (windowHandle() && windowHandle()->screen()) {
                dpr = windowHandle()->screen()->devicePixelRatio();
            }

            QPixmap scaled = Globals::makeHighDpiPixmap(
                m_pixmap,
                imageRect.size(),
                dpr
            );

            QSize drawSize = scaled.deviceIndependentSize().toSize();

            QRect drawRect(
                imageRect.x() + (imageRect.width() - drawSize.width()) / 2,
                imageRect.y() + (imageRect.height() - drawSize.height()) / 2,
                drawSize.width(),
                drawSize.height()
            );

            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(
                imageRect.adjusted(-4, -4, 4, 4),
                qMax(8, int(radius * 0.7)),
                qMax(8, int(radius * 0.7))
            );

            painter.drawPixmap(drawRect.topLeft(), scaled);
        }
        else {
            painter.setPen(QPen(QColor(210, 225, 240), 1));
            painter.setBrush(QColor(242, 247, 253));
            painter.drawRoundedRect(
                imageRect,
                qMax(8, int(radius * 0.7)),
                qMax(8, int(radius * 0.7))
            );

            QFont iconFont = painter.font();
            iconFont.setBold(true);
            iconFont.setPixelSize(qMax(12, int(h * 0.18)));
            painter.setFont(iconFont);
            painter.setPen(QColor(140, 160, 180));
            painter.drawText(imageRect, Qt::AlignCenter, "IMG");
        }

        textLeft = imageRect.right() + spacing;
    }

    // 描述文字区域
    QRect descRect(textLeft, 0, width() - textLeft - rightReserved, height());

    QFont descFont = painter.font();
    descFont.setBold(false);
    descFont.setPixelSize(qMax(12, int(h * 0.25)));
    painter.setFont(descFont);
    painter.setPen(QColor(35, 35, 35));

    QFontMetrics descFm(descFont);
    QString elidedDesc = descFm.elidedText(m_description, Qt::ElideRight, descRect.width());
    painter.drawText(descRect, Qt::AlignVCenter | Qt::AlignLeft, elidedDesc);
}