#include "AntBaseListView.h"

AntBaseListView::AntBaseListView(QWidget* parent)
	: QListView(parent)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// 自定义样式
	m_style = new AntBaseListViewStyle(style());
	setStyle(m_style);

	// 自定义 Item Delegate
	m_itemDele = new AntBaseListItemDelegate(this);
	setItemDelegate(m_itemDele);
}

AntBaseListView::~AntBaseListView()
{
}