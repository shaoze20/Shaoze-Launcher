#include "SlideStackedWidget.h"
#include <QParallelAnimationGroup>

SlideStackedWidget::SlideStackedWidget(QWidget* parent)
	: QStackedWidget(parent)
{
	setObjectName("SlideStackedWidget");
	setFrameShape(QFrame::NoFrame);
	setContentsMargins(0, 0, 0, 0);
}

SlideStackedWidget::~SlideStackedWidget()
{
}

void SlideStackedWidget::slideToPage(QWidget* nextWidget, int duration, std::function<void()> onFinished)
{
	if (isAnimating || nextWidget == currentWidget()) return;
	isAnimating = true;

	QWidget* currentPage = currentWidget();
	int w = width();
	int h = height();

	currentPage->resize(w, h);
	nextWidget->resize(w, h);

	QPoint nextStart(50, 0);
	QPoint nextEnd(0, 0);

	nextWidget->move(nextStart);
	nextWidget->show();

	// 动画效果
	QEasingCurve curve(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(QPointF(0.25, 0.1), QPointF(0.25, 1.0), QPointF(1.0, 1.0));

	QPropertyAnimation* anim = new QPropertyAnimation(nextWidget, "pos");
	anim->setDuration(duration);
	anim->setStartValue(nextStart);
	anim->setEndValue(nextEnd);
	anim->setEasingCurve(curve);

	connect(anim, &QPropertyAnimation::valueChanged, this, [this](const QVariant&) {
		this->update();
		});

	connect(anim, &QPropertyAnimation::finished, this, [=]()
		{
			setCurrentWidget(nextWidget);
			currentPage->move(0, 0);
			isAnimating = false;
			if (onFinished) onFinished();
		});

	currentPage->hide();

	QElapsedTimer* animTimer = new QElapsedTimer();
	animTimer->start();

	QTimer* smoothTimer = new QTimer(anim);
	int interval = 1000 / screen()->refreshRate();
	smoothTimer->setInterval(interval);
	connect(smoothTimer, &QTimer::timeout, anim, [anim, animTimer]() {
		if (anim->state() == QAbstractAnimation::Running) {
			int elapsed = static_cast<int>(animTimer->elapsed());
			if (elapsed > anim->duration()) elapsed = anim->duration();
			anim->setCurrentTime(elapsed);
		}
		});
	smoothTimer->start();
	anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SlideStackedWidget::slideFromBottomToTop(QWidget* nextWidget, int duration, std::function<void()> onFinished)
{
	if (!nextWidget) return;
	if (nextWidget == currentWidget())
	{
		if (onFinished) onFinished();
		return;
	}

	isAnimating = true;
	QWidget* currentPage = currentWidget();
	int w = width();
	int h = height();

	currentPage->resize(w, h);
	nextWidget->resize(w, h);

	QPoint nextStart(0, static_cast<int>(0.15 * h));
	QPoint nextEnd(0, 0);

	nextWidget->move(nextStart);
	nextWidget->show();

	// 效果
	QEasingCurve curve(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(QPointF(0.25, 0.1), QPointF(0.25, 1.0), QPointF(1.0, 1.0));

	// 动画
	QPropertyAnimation* animNextPos = new QPropertyAnimation(nextWidget, "pos");
	animNextPos->setDuration(duration);
	animNextPos->setStartValue(nextStart);
	animNextPos->setEndValue(nextEnd);
	animNextPos->setEasingCurve(curve);

	connect(animNextPos, &QPropertyAnimation::valueChanged, this, [this](const QVariant&) {
		this->update();
		});

	connect(animNextPos, &QPropertyAnimation::finished, this, [=]()
		{
			setCurrentWidget(nextWidget);
			nextWidget->move(0, 0);
			isAnimating = false;
			if (onFinished) onFinished();
		});

	currentPage->hide();

	animNextPos->start(QAbstractAnimation::DeleteWhenStopped);

	QElapsedTimer* animTimer = new QElapsedTimer();
	animTimer->start();
	QTimer* smoothTimer = new QTimer(animNextPos);
	int interval = 1000 / screen()->refreshRate();
	smoothTimer->setInterval(interval);
	connect(smoothTimer, &QTimer::timeout, animNextPos, [animNextPos, animTimer]() {
		if (animNextPos->state() == QAbstractAnimation::Running) {
			int elapsed = static_cast<int>(animTimer->elapsed());
			if (elapsed > animNextPos->duration()) elapsed = animNextPos->duration();
			animNextPos->setCurrentTime(elapsed);
		}
		});
	smoothTimer->start();
}