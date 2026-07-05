#include "PopupPanel.h"

PopupPanel::PopupPanel(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

void PopupPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);

    QPainterPath path;
    path.addRoundedRect(r, 10, 10);
    painter.fillPath(path, QColor(60, 60, 60));
}