#include "LinkLabel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QFontMetrics>
#include <QCursor>

LinkLabel::LinkLabel(const QString& text, const QString& url, QWidget* parent)
	: QWidget(parent),
	m_text(text),
	m_url(url)
{
	setAttribute(Qt::WA_StyledBackground, false);
	setAutoFillBackground(false);
	setCursor(Qt::PointingHandCursor);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void LinkLabel::setText(const QString& text)
{
	if (m_text == text)
		return;

	m_text = text;
	updateGeometry();
	update();
}

QString LinkLabel::text() const
{
	return m_text;
}

void LinkLabel::setUrl(const QString& url)
{
	m_url = url;
}

QString LinkLabel::url() const
{
	return m_url;
}

void LinkLabel::setTextColor(const QColor& color)
{
	m_textColor = color;
	update();
}

QSize LinkLabel::sizeHint() const
{
	QFontMetrics fm(font());
	int w = fm.horizontalAdvance(m_text);
	int h = fm.height();
	return QSize(w + 4, h + 4);
}

void LinkLabel::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setPen(m_textColor);
	painter.setFont(font());

	QRect textRect = rect();
	painter.drawText(textRect, Qt::AlignCenter, m_text);

	QFontMetrics fm(font());
	int textWidth = fm.horizontalAdvance(m_text);
	int textHeight = fm.height();

	int x = (width() - textWidth) / 2;
	int y = (height() + textHeight) / 2 - 1;

	painter.drawLine(x, y, x + textWidth, y);
}

void LinkLabel::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && !m_url.isEmpty()) {
		QDesktopServices::openUrl(QUrl(m_url));
	}
	QWidget::mousePressEvent(event);
}

void LinkLabel::enterEvent(QEnterEvent* event)
{
	Q_UNUSED(event);
	m_hovered = true;
	update();
}

void LinkLabel::leaveEvent(QEvent* event)
{
	Q_UNUSED(event);
	m_hovered = false;
	update();
}