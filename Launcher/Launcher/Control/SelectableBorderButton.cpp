#include "SelectableBorderButton.h"

SelectableBorderButton::SelectableBorderButton(const QString& text, QWidget* parent)
    : QAbstractButton(parent)
{
    setText(text);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(40);
}

QSize SelectableBorderButton::sizeHint() const
{
    return QSize(140, 40);
}

QSize SelectableBorderButton::minimumSizeHint() const
{
    return QSize(100, 42);
}

void SelectableBorderButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF r = rect().adjusted(1, 1, -1, -1);
    const qreal radius = 10.0;

    QColor bgColor = isChecked()
        ? QColor(232, 232, 232)
        : QColor(242, 242, 242);

    QColor borderColor = isChecked()
        ? QColor(0, 0, 0)
        : QColor(120, 120, 120);

    int borderWidth = isChecked() ? 3 : 2;

    if (isDown()) {
        borderColor = QColor(0, 0, 0);
        bgColor = QColor(225, 225, 225);
    }
    else if (underMouse() && !isChecked()) {
        borderColor = QColor(70, 70, 70);
    }

    painter.setPen(QPen(borderColor, borderWidth));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(r, radius, radius);

    QFont font = this->font();
    font.setPixelSize(15);
    font.setWeight(isChecked() ? QFont::Bold : QFont::Medium);
    painter.setFont(font);
    painter.setPen(QColor(20, 20, 20));
    painter.drawText(rect(), Qt::AlignCenter, text());
}