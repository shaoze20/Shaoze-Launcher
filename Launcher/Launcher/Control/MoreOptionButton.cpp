#include "MoreOptionButton.h"
#include <QPainter>
#include <QPixmap>

MoreOptionButton::MoreOptionButton(QWidget* parent)
	: QPushButton(parent)
{
	setCursor(Qt::PointingHandCursor);
	setFixedSize(50, 50);
	setAttribute(Qt::WA_Hover);

	// 创建弹出面板
	m_popup = new PopupPanel(nullptr);
	m_popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	m_popup->setAttribute(Qt::WA_TranslucentBackground);
	m_popup->installEventFilter(this);

	m_layout = new QVBoxLayout(m_popup);
	m_layout->setContentsMargins(5, 5, 5, 5);
	m_layout->setSpacing(3);

	m_popup->hide();

	// 透明度效果
	m_opacityEffect = new QGraphicsOpacityEffect(m_popup);
	m_popup->setGraphicsEffect(m_opacityEffect);

	// 位置动画
	m_posAnim = new QPropertyAnimation(m_popup, "pos", this);
	m_posAnim->setDuration(180);
	m_posAnim->setEasingCurve(QEasingCurve::OutCubic);

	// 透明度动画
	m_opacityAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
	m_opacityAnim->setDuration(150);
	m_opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
}

void MoreOptionButton::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	QColor bgColor = m_hovered ? QColor(45, 45, 45) : QColor(60, 60, 60);	// 背景颜色 
	QColor borderColor = m_hovered ? QColor(255, 255, 255, 160) : QColor(200, 200, 200, 120);	// 灰白色边框

	// 路径
	QRectF r = rect().adjusted(1, 1, -1, -1);
	QPainterPath path;
	path.addEllipse(r);
	painter.fillPath(path, bgColor);

	// 边框
	QPen pen(borderColor);
	pen.setWidthF(1.0);

	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(path);

	// 图标
	QPixmap icon(":/Launcher/Imgs/MoreItem.svg");
	if (!icon.isNull()) {
		QSize iconSize(15, 25);
		QPoint center = rect().center();

		QRect iconRect(
			center.x() - iconSize.width() / 2,
			center.y() - iconSize.height() / 2,
			iconSize.width(),
			iconSize.height()
		);

		painter.drawPixmap(iconRect, icon);
	}
}

void MoreOptionButton::enterEvent(QEnterEvent* event)
{
	m_hovered = true;
	update();

	showPopup(); // 悬停弹出

	QPushButton::enterEvent(event);
}

void MoreOptionButton::leaveEvent(QEvent* event)
{
	m_hovered = false;
	update();

	// 延迟隐藏
	QTimer::singleShot(150, this, [this]() {
		if (!underMouse() && !m_popup->underMouse()) {
			hidePopup();
		}
		});

	QPushButton::leaveEvent(event);
}

bool MoreOptionButton::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == m_popup) {
		if (event->type() == QEvent::Leave) {
			QTimer::singleShot(150, this, [this]() {
				if (!underMouse() && !m_popup->underMouse()) {
					hidePopup();
				}
				});
		}
	}
	return QPushButton::eventFilter(obj, event);
}

void MoreOptionButton::addMenuButton(QPushButton* btn)
{
	btn->setParent(m_popup);
	btn->setCursor(Qt::PointingHandCursor);

	m_layout->addWidget(btn);

	// 自适应高度
	m_popup->adjustSize();
}

void MoreOptionButton::showPopup()
{
	if (!m_popup) return;

	m_popup->adjustSize();

	QPoint globalPos = mapToGlobal(QPoint(0, 0));

	int x = globalPos.x() + width() / 2 - m_popup->width() / 2;

	int finalY = globalPos.y() - m_popup->height() - 20;
	int startY = finalY + 25; // 从下往上滑

	QPoint startPos(x, startY);
	QPoint endPos(x, finalY);

	// 初始状态
	m_popup->move(startPos);
	m_popup->show();

	// 停止旧动画
	m_posAnim->stop();
	m_opacityAnim->stop();

	// 位置动画
	m_posAnim->setStartValue(startPos);
	m_posAnim->setEndValue(endPos);

	// 透明度动画
	m_opacityEffect->setOpacity(0.0);
	m_opacityAnim->setStartValue(0.0);
	m_opacityAnim->setEndValue(1.0);

	// 启动动画
	m_posAnim->start();
	m_opacityAnim->start();
}

void MoreOptionButton::hidePopup(bool force)
{
	if (!m_popup) return;

	if (!force && m_popup->underMouse()) return;

	m_opacityAnim->stop();
	m_opacityAnim->setStartValue(m_opacityEffect->opacity());
	m_opacityAnim->setEndValue(0.0);

	connect(m_opacityAnim, &QPropertyAnimation::finished, this, [this]() {
		if (m_opacityEffect->opacity() == 0.0)
			m_popup->hide();
		});

	m_opacityAnim->start();
}