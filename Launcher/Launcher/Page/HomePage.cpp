#include "HomePage.h"
#include <QVBoxLayout>
#include <QLabel>

HomePage::HomePage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("HomePage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    // 背景图
    m_bgOriginal.load(DesignSystem::instance()->homeVideoFilePath());

    // 时间控件
    m_clock = new GlassClockWidget(this);
    m_clock->resize(300, 130); // 控件大小可调
    m_clock->show();
}

void HomePage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 8;
    QPainterPath path;
    QRect r = rect();

    path.moveTo(r.topLeft());
    path.lineTo(r.topRight() - QPoint(radius, 0));
    path.quadTo(r.topRight(), r.topRight() + QPoint(0, radius));
    path.lineTo(r.bottomRight() - QPoint(0, radius));
    path.quadTo(r.bottomRight(), r.bottomRight() - QPoint(radius, 0));
    path.lineTo(r.bottomLeft());
    path.lineTo(r.topLeft());

    painter.setClipPath(path);

    // 背景图
    if (!m_bgOriginal.isNull()) {
        painter.drawPixmap(rect(), m_bgOriginal);
    }

    // 水印
    QString watermark = "Shaoze  Launcher";

    QFont font = painter.font();
    font.setPointSize(6);
    painter.setFont(font);

    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(watermark);
    int textH = fm.height();

    int margin = 5;
    int x = width() - textW - margin;
    int y = height() - margin;

    painter.setPen(QColor(255, 255, 255, 120)); // 半透明白
    painter.drawText(x, y, watermark);
}
void HomePage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (m_clock)
    {
        int x = (width() - m_clock->width()) / 2;
        int y = 0; // 距离顶部 20px（可调）

        m_clock->move(x, 0);
    }
}