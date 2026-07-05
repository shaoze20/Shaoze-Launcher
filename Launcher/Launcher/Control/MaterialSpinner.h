#pragma once

#include <QWidget>

class MaterialSpinner : public QWidget
{
	Q_OBJECT
		Q_PROPERTY(qreal dashLength READ dashLength WRITE setDashLength)
		Q_PROPERTY(qreal dashOffset READ dashOffset WRITE setDashOffset)
		Q_PROPERTY(qreal angle READ angle WRITE setAngle)

public:
	explicit MaterialSpinner(QSize size, int lineWidth, QColor lineColor, QWidget* parent);

	qreal dashLength() const;
	void setDashLength(qreal length);

	qreal dashOffset() const;
	void setDashOffset(qreal offset);

	qreal angle() const;
	void setAngle(qreal angle);

protected:
	void paintEvent(QPaintEvent*) override;

private:
	void initAnimations();

private:
	qreal m_dashLength;
	qreal m_dashOffset;
	qreal m_angle;
	int m_lineWidth;
	int m_length;		// 矩形边长
	QColor m_lineColor;
};
