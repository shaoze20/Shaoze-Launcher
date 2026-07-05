#pragma once
#include <QString>
#include <QColor>

namespace StyleSheet {

	// 垂直方向列表视图
	inline QString vListViewQss(const QColor& handleColor)
	{
		return QString(R"(
		QListView {
			background-color: transparent;
			border: none;
		}

		QScrollBar:vertical {
			width: 6px;
			background: transparent;
			border: none;
		}

		QScrollBar::handle:vertical {
			background: %1;
			border-radius: 3px;
		}

		QScrollBar::sub-line:vertical,
		QScrollBar::add-line:vertical {
			height: 0px;
			border: none;
			background: none;
		}

		QScrollBar::add-page:vertical,
		QScrollBar::sub-page:vertical {
			background: none;
		})")
			.arg(handleColor.name());
	}

	inline QString background(const QColor& c)
	{
		return QString("background-color: %1;").arg(c.name());
	}
}