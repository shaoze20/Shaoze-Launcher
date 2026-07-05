#pragma once

#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QElapsedTimer>

class SlideStackedWidget : public QStackedWidget
{
	Q_OBJECT

public:
	SlideStackedWidget(QWidget* parent = nullptr);
	~SlideStackedWidget();

	bool isAnimationRunning() { return isAnimating; };
	void slideToPage(QWidget* nextWidget, int duration = 250, std::function<void()> onFinished = nullptr);
	void slideFromBottomToTop(QWidget* nextWidget, int duration, std::function<void()> onFinished);

private:

	bool isAnimating = false;
};
