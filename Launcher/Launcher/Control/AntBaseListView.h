#pragma once

#include <QListView>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QProxyStyle>
#include <QPainterPath>
#include <QStandardItemModel>

// 自定义 Item Delegate 来控制 ListItem 的大小
class AntBaseListItemDelegate : public QStyledItemDelegate
{
public:
	explicit AntBaseListItemDelegate(QObject* parent = nullptr)
		: QStyledItemDelegate(parent)
	{
		// 选中的背景颜色
		m_curIdxBgColor = QColor(220, 220, 220);
		m_curIdxBgColor.setAlpha(180);

		// 悬停的背景色
		m_hoverColor = QColor(230, 230, 230);
		m_hoverColor.setAlpha(80);

		// 默认背景色
		m_bgColor = QColor(255, 255, 255, 0);

		// 文本颜色
		m_textColor = QColor(0, 0, 0);
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		Q_UNUSED(index);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		QRectF rect = option.rect.adjusted(2, 2, -2, -2);
		const qreal borderRadius = 6.0;

		bool isHovered = option.state & QStyle::State_MouseOver;
		bool isSelected = option.state & QStyle::State_Selected;

		QColor bg;
		if (isSelected) {
			bg = m_curIdxBgColor;
		}
		else if (isHovered) {
			bg = m_hoverColor;
		}
		else {
			bg = m_bgColor;
		}

		painter->setPen(Qt::NoPen);
		painter->setBrush(bg);
		painter->drawRoundedRect(rect, borderRadius, borderRadius);

		painter->restore();
	}
protected:
	void updateBaseStyle()
	{
		m_curIdxBgColor = QColor(220, 220, 220);
		m_curIdxBgColor.setAlpha(180);

		m_hoverColor = QColor(230, 230, 230);
		m_hoverColor.setAlpha(80);

		m_bgColor = QColor(255, 255, 255, 0);

		m_textColor = QColor(230, 230, 230);
	}
protected:
	QColor m_bgColor;					// 默认背景色
	QColor m_hoverColor;				// 悬停的背景色
	QColor m_curIdxBgColor;				// 当前选中项的背景颜色
	mutable QColor m_textColor;			// 文本颜色
};

class AntBaseListViewStyle : public QProxyStyle
{
public:
	AntBaseListViewStyle(QStyle* baseStyle = nullptr)
		: QProxyStyle(baseStyle)
	{
		m_bgColor = QColor(255, 255, 255, 0);
		m_borderColor = QColor(200, 200, 200);
	}

	void drawControl(ControlElement element, const QStyleOption* option,
		QPainter* painter, const QWidget* widget) const override
	{
		if (element == CE_ShapedFrame && widget && qobject_cast<const QListView*>(widget))
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing);

			int margin = 3;
			int radius = 0;
			QRect rect = option->rect.adjusted(margin, margin, -margin, -margin);

			// 再绘制背景和边框
			painter->setPen(QPen(m_borderColor, 1.5));
			painter->setBrush(m_bgColor);
			painter->drawRoundedRect(rect, radius, radius);

			painter->restore();
			return;
		}
		QProxyStyle::drawControl(element, option, painter, widget);
	}

	void updateBaseStyle()
	{
		m_bgColor = QColor(255, 255, 255, 0);
		m_borderColor = QColor(200, 200, 200);
	}
private:
	QColor m_bgColor;
	QColor m_borderColor;
};

class AntBaseListView : public QListView
{
	Q_OBJECT

public:
	AntBaseListView(QWidget* parent);
	~AntBaseListView();
protected:
	AntBaseListViewStyle* m_style;
	AntBaseListItemDelegate* m_itemDele;
};
