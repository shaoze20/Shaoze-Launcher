#include "MaterialSpinner.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>

MaterialSpinner::MaterialSpinner(QSize size, int lineWidth, QColor lineColor, QWidget* parent)
	: QWidget(parent),
	m_dashLength(0.1),
	m_dashOffset(0),
	m_angle(0),
	m_lineWidth(lineWidth),
	m_lineColor(lineColor)
{
	// 注意宽高必须相等
	m_length = qMin(size.width(), size.height());
	setFixedSize(m_length, m_length);
	initAnimations();

	m_lineColor = lineColor;
    update();
}

qreal MaterialSpinner::dashLength() const
{
	return m_dashLength;
}

void MaterialSpinner::setDashLength(qreal length)
{
	if (qFuzzyCompare(m_dashLength, length))
		return;
	m_dashLength = length;
	update();
}

qreal MaterialSpinner::dashOffset() const
{
	return m_dashOffset;
}

void MaterialSpinner::setDashOffset(qreal offset)
{
	if (qFuzzyCompare(m_dashOffset, offset))
		return;
	m_dashOffset = offset;
	update();
}

qreal MaterialSpinner::angle() const
{
	return m_angle;
}

void MaterialSpinner::setAngle(qreal angle)
{
	if (qFuzzyCompare(m_angle, angle))
		return;
	m_angle = angle;
	update();
}

void MaterialSpinner::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const int size = qMin(width(), height());

	painter.translate(width() / 2.0, height() / 2.0);
	painter.rotate(m_angle);

	QPen pen(m_lineColor);
	pen.setWidthF(m_lineWidth);
	pen.setCapStyle(Qt::RoundCap);

	// 设置动态虚线 pattern
	QVector<qreal> pattern;
	pattern << m_dashLength * size / 50 << 36 * size / 50;	// 36这个值自己调整 合适即可
	pen.setDashPattern(pattern);
	pen.setDashOffset(m_dashOffset * size / 50);

	painter.setPen(pen);

	int radius = (size - m_lineWidth) / 2;
	painter.drawEllipse(QPointF(0, 0), radius, radius);
}

void MaterialSpinner::initAnimations()
{
	QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

	// dashLength 动画
	QPropertyAnimation* dashLengthAnim = new QPropertyAnimation(this, "dashLength");
	dashLengthAnim->setDuration(2100);
	dashLengthAnim->setStartValue(3.0);
	dashLengthAnim->setKeyValueAt(0.15, 6.0);
	dashLengthAnim->setKeyValueAt(0.6, 20);
	dashLengthAnim->setKeyValueAt(0.7, 20);
	dashLengthAnim->setEndValue(25);
	dashLengthAnim->setEasingCurve(QEasingCurve::InOutSine);

	// dashOffset 动画
	QPropertyAnimation* dashOffsetAnim = new QPropertyAnimation(this, "dashOffset");
	dashOffsetAnim->setDuration(2100);
	dashOffsetAnim->setStartValue(0);
	dashOffsetAnim->setKeyValueAt(0.15, 0);
	dashOffsetAnim->setKeyValueAt(0.6, -7);
	dashOffsetAnim->setKeyValueAt(0.7, -7);
	dashOffsetAnim->setEndValue(-38);
	dashOffsetAnim->setEasingCurve(QEasingCurve::InOutSine);

	// angle 动画
	QPropertyAnimation* angleAnim = new QPropertyAnimation(this, "angle");
	angleAnim->setDuration(2100);
	angleAnim->setStartValue(0);
	angleAnim->setEndValue(720);
	angleAnim->setEasingCurve(QEasingCurve::Linear);

	group->addAnimation(dashLengthAnim);
	group->addAnimation(dashOffsetAnim);
	group->addAnimation(angleAnim);

	group->setLoopCount(-1);
	group->start();
}