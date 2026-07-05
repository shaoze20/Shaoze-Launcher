#pragma once
#include <QWidget>
#include <qpainterpath.h>
#include <qscrollarea.h>
#include <QPainter>
#include <QGridLayout>
#include <QToolButton>

#include "../Control/CustomToolButton.h"
#include "../Control/SmoothScrollBar.h"
#include "../Style/StyleSheet.h"

class EmojiPanel : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiPanel(QWidget* parent = nullptr);

public:
    void setArrowCenterX(int x) { m_arrowCenterX = x; update(); }
signals:
    void emojiClicked(const QString& emoji);
    void panelClosed();

protected:
    void paintEvent(QPaintEvent*) override;
    void hideEvent(QHideEvent* e) override;

private:
    int m_radius = 10;      // 圆角半径
    int m_arrowW = 14;      // 三角宽
    int m_arrowH = 8;       // 三角高
    int m_arrowCenterX = -1;
    QScrollArea* m_scroll = nullptr;
    int m_fixedContentH = 290;// 你想要固定显示的内容高度
};