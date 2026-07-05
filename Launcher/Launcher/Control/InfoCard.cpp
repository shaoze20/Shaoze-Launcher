#include "InfoCard.h"

InfoCard::InfoCard(const QString& title, const QString& content, QWidget* parent)
    : QFrame(parent)
{
    setObjectName("infoCard");
    setStyleSheet(R"(
        QFrame#infoCard {
            background: white;
            border: 1px solid #E5E5E5;
            border-radius: 12px;
        }
        QLabel#cardTitle {
            color: #222222;
            font-size: 16px;
            font-weight: 600;
            background: transparent;
        }
        QLabel#cardContent {
            color: #555555;
            font-size: 14px;
            background: transparent;
        }
    )");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(18, 16, 18, 16);
    m_layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("cardTitle");

    m_contentLabel = new QLabel(this);
    m_contentLabel->setObjectName("cardContent");
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(m_contentLabel);

    setTitle(title);
    setContent(content);
}

void InfoCard::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

void InfoCard::setContent(const QString& content)
{
    if (m_contentLabel) {
        m_contentLabel->setText(content.isEmpty() ? QStringLiteral("未检测到信息") : content);
    }
}

QString InfoCard::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

QString InfoCard::content() const
{
    return m_contentLabel ? m_contentLabel->text() : QString();
}