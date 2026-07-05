#include "CustomGlowButton.h"
#include <QPainter>
#include <QMouseEvent>

class TooltipWidget : public QWidget {
public:
    explicit TooltipWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);

        m_label = new QLabel(this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setStyleSheet("color: white;");

        // 初始化透明度为 0
        setWindowOpacity(0);

        // 创建动画对象
        m_animation = new QPropertyAnimation(this, "windowOpacity");
        m_animation->setDuration(100); // 0.2 秒

        m_posAnimation = new QPropertyAnimation(this, "pos");
        m_posAnimation->setDuration(120);
        m_posAnimation->setEasingCurve(QEasingCurve::OutCubic);
    }

    void setText(const QString& text) {
        m_label->setText(text);
        m_label->adjustSize();
        QFont font = m_label->font();
        font.setPointSize(10);
        m_label->setFont(font);

        int triangleWidth = 10;
        int paddingX = 10;  // 左右更宽
        int paddingY = 6;   // 上下更高

        int w = m_label->width() + paddingX * 2 + triangleWidth;
        int h = m_label->height() + paddingY * 2;
        resize(w, h);
        m_label->move(triangleWidth + paddingX + 1, paddingY - 1);
    }

    void showAnimated(const QPoint& targetPos) {
        m_targetPos = targetPos;

        int offset = 30; // 向右展开距离

        QPoint startPos = QPoint(targetPos.x() - offset, targetPos.y());

        // 初始状态
        move(startPos);
        setWindowOpacity(0.0);

        show();
        raise();

        // 停止旧动画
        m_animation->stop();
        m_posAnimation->stop();

        // 透明度动画
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);

        // 位置动画
        m_posAnimation->setStartValue(startPos);
        m_posAnimation->setEndValue(targetPos);

        // 启动
        m_animation->start();
        m_posAnimation->start();
    }

    void hideAnimated() {
        m_animation->stop();
        m_posAnimation->stop();

        int offset = 10;

        QPoint endPos = QPoint(m_targetPos.x() - offset, m_targetPos.y());

        m_animation->setStartValue(windowOpacity());
        m_animation->setEndValue(0.0);

        m_posAnimation->setStartValue(pos());
        m_posAnimation->setEndValue(endPos);

        connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
            if (windowOpacity() == 0.0) {
                hide();
            }
            });

        m_animation->start();
        m_posAnimation->start();
    }

protected:
    void paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 参数
        int triangleWidth = 15;
        int triangleHeight = 8;
        int rectRadius = 12;
        int borderWidth = 1;

        QColor bgColor(60, 60, 60);
        QColor borderColor(150, 150, 150);

        // 矩形区域
        int rectX = triangleWidth;
        int rectY = borderWidth;
        int rectW = width() - triangleWidth - borderWidth;
        int rectH = height() - 2 * borderWidth;

        QRect rect(rectX, rectY, rectW, rectH);

        int centerY = height() / 2;

        QPainterPath path;
        path.moveTo(rectX, centerY - triangleHeight / 2);
        path.quadTo(2, centerY, rectX, centerY + triangleHeight / 2);
        path.lineTo(rectX, rect.bottom() - rectRadius);
        path.quadTo(rectX, rect.bottom(), rectX + rectRadius, rect.bottom());
        path.lineTo(rect.right() - rectRadius, rect.bottom());
        path.quadTo(rect.right(), rect.bottom(), rect.right(), rect.bottom() - rectRadius);
        path.lineTo(rect.right(), rect.top() + rectRadius);
        path.quadTo(rect.right(), rect.top(), rect.right() - rectRadius, rect.top());
        path.lineTo(rectX + rectRadius, rect.top());
        path.quadTo(rectX, rect.top(), rectX, rect.top() + rectRadius);
        path.closeSubpath();

        // 填充 
        painter.setBrush(bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawPath(path);

        // 描边
        QPen pen(borderColor);
        pen.setWidth(borderWidth);
        pen.setJoinStyle(Qt::RoundJoin); // 防止连接处尖角
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

private:
    QLabel* m_label;
    QPropertyAnimation* m_animation;
    QPropertyAnimation* m_posAnimation;
    QPoint m_targetPos;
};

CustomGlowButton::CustomGlowButton(QWidget* parent)
    : QPushButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(40, 40);

    m_iconAnimSize = m_iconSize;

    m_iconAnim = new QPropertyAnimation(this, "iconAnimSize");
    m_iconAnim->setDuration(220);
    m_iconAnim->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QPushButton::toggled, this, [this](bool checked) {

        m_borderOpacity = checked ? 1.0 : 0.0;

        QSize targetSize;

        if (checked)
            targetSize = m_iconSize * 1.15;
        else
            targetSize = m_iconSize;

        m_iconAnim->stop();
        m_iconAnim->setStartValue(m_iconAnimSize);
        m_iconAnim->setEndValue(targetSize);
        m_iconAnim->start();
        });
}

void CustomGlowButton::setIconPixmap(const QPixmap& pix)
{
    m_icon = pix;
    update();
}

void CustomGlowButton::setIconSize(const QSize& size)
{
    m_iconSize = size;
    m_iconAnimSize = isChecked()
        ? QSize(int(size.width() * 1.15), int(size.height() * 1.15))
        : size;
    update();
}

void CustomGlowButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF buttonRect = rect().adjusted(1, 1, -1, -1);

    // 选中背景
    if (m_borderOpacity > 0.0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hoverColor);
        painter.drawRoundedRect(buttonRect, 8, 8);
    }

    // 背景
    if (m_hovered && !isChecked()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hoverColor);
        painter.drawRoundedRect(buttonRect, 8, 8);
    }

    // 图标
    if (!m_icon.isNull() && !m_iconSize.isEmpty()) {
        qreal dpr = devicePixelRatioF();
        if (windowHandle() && windowHandle()->screen()) {
            dpr = windowHandle()->screen()->devicePixelRatio();
        }

        QPixmap hdpiPixmap = Globals::makeHighDpiPixmap(m_icon, m_iconAnimSize, dpr);

        QSizeF logicalDrawSize = hdpiPixmap.deviceIndependentSize();
        QPoint center = rect().center();

        QRectF iconRect(
            center.x() - logicalDrawSize.width() / 2.0,
            center.y() - logicalDrawSize.height() / 2.0,
            logicalDrawSize.width(),
            logicalDrawSize.height()
        );

        QPainterPath clipPath;
        clipPath.addRoundedRect(iconRect, 8.0, 8.0);

        painter.save();
        painter.setClipPath(clipPath);
        painter.drawPixmap(iconRect.topLeft(), hdpiPixmap);
        painter.restore();
    }
}

void CustomGlowButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();

    if (!m_tooltipText.isEmpty() && m_tooltipWidget) {
        QPoint globalPos = mapToGlobal(QPoint(width(), 0));

        int x = globalPos.x() + 5;
        int y = globalPos.y() + (height() - m_tooltipWidget->height()) / 2;
        m_tooltipTargetPos = QPoint(x, y);

        if (m_tooltipTimer->isActive()) {
            m_tooltipTimer->stop();
        }

        // 延迟 500ms 显示
        m_tooltipTimer->start(500);
    }

    QPushButton::enterEvent(event);
}

void CustomGlowButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();

    if (m_tooltipTimer->isActive()) {
        m_tooltipTimer->stop();
    }

    if (m_tooltipWidget) {
        static_cast<TooltipWidget*>(m_tooltipWidget)->hideAnimated();
    }

    QPushButton::leaveEvent(event);
}

void CustomGlowButton::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    update();
}

void CustomGlowButton::setTooltipText(const QString& text)
{
    m_tooltipText = text;

    if (m_tooltipText.isEmpty()) {
        if (m_tooltipWidget) m_tooltipWidget->hide();
        return;
    }

    if (!m_tooltipWidget) {
        m_tooltipWidget = new TooltipWidget(nullptr);

        m_tooltipTimer = new QTimer(this);
        m_tooltipTimer->setSingleShot(true);

        connect(m_tooltipTimer, &QTimer::timeout, [this]() {
            if (m_tooltipWidget) {
                static_cast<TooltipWidget*>(m_tooltipWidget)->showAnimated(m_tooltipTargetPos);
            }
            });
    }

    static_cast<TooltipWidget*>(m_tooltipWidget)->setText(m_tooltipText);
}

void CustomGlowButton::setBorderOpacity(qreal value)
{
    m_borderOpacity = qBound<qreal>(0.0, value, 1.0);
    update();
}