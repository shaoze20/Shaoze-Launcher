#include "AntMessageManager.h"
#include <QApplication>
#include "DesignSystem.h"

AntMessageManager* AntMessageManager::m_instance = nullptr;

AntMessageManager::AntMessageManager(QWidget* parent)
	: QWidget(parent)
{
}

AntMessageManager::~AntMessageManager()
{
}

QRect AntMessageManager::getScreenGeometry() const
{
	// Qt5: QDesktopWidget；Qt6: QScreen
#ifdef QT_VERSION_MAJOR
#if QT_VERSION_MAJOR >= 6
	QScreen* screen = QGuiApplication::primaryScreen();
	return screen->geometry();
#else
	QDesktopWidget* desktop = QApplication::desktop();
	return desktop->screenGeometry(desktop->primaryScreen());
#endif
#else
	QDesktopWidget* desktop = QApplication::desktop();
	return desktop->screenGeometry(desktop->primaryScreen());
#endif
}

AntMessageManager* AntMessageManager::instance()
{
	if (!m_instance)
	{
		m_instance = new AntMessageManager();
	}
	return m_instance;
}

void AntMessageManager::showMessage(AntMessage::Type type, const QString& message, int offsetY, int msgDuration) 
{
	m_offsetY = offsetY;
	QWidget* mainWindow = DesignSystem::instance()->getMainWindow();
	AntMessage* msg = new AntMessage(mainWindow, type, message);
	msgHeight = msg->height();
	connect(msg, &AntMessage::requestExit, this, &AntMessageManager::onMessageRequestExit);
	connect(msg, &AntMessage::exitAnimationFinished, this, &AntMessageManager::onMessageExitFinished);

	// 稳妥地计算 Y：累加所有当前消息的高度 + 间距
	int y = spacingY + m_offsetY;
	for (AntMessage* m : m_messages)
	{
		y += msgHeight + spacingY;
	}

	// 居中计算
	int x = (mainWindow->width() - msg->width()) / 2;

	// 动画
	QPropertyAnimation* opacityAnim = new QPropertyAnimation(msg, "customOpacity");
	opacityAnim->setDuration(animDuration);
	opacityAnim->setEasingCurve(QEasingCurve::OutSine);
	opacityAnim->setStartValue(0.0);
	opacityAnim->setEndValue(1.0);

	if (m_isBatchAnimating)
	{
		msg->move(x, y - spacingY - msg->height());
	}
	else
	{
		msg->move(QPoint(x, y));
	}

	m_messages.append(msg);
	msg->show();
	opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
	msg->startDisplayTimer(msgDuration);
}

void AntMessageManager::onMessageRequestExit(AntMessage* msg)
{
	if (isAnimating)
	{
		// 当前已有动画在执行，先不处理
		return;
	}

	if (!m_messages.isEmpty() && msg == m_messages.first())
	{
		// 队首消息请求退出，开始退出动画
		startExitAnimation();
	}
	// 不是队首消息请求退出，忽略，等待队首退出
}

void AntMessageManager::startExitAnimation()
{
	if (m_messages.isEmpty()) return;

	isAnimating = true;
	AntMessage* firstMsg = m_messages.first();
	firstMsg->setIsExit(true);

	// 首条消息滑出动画
	QParallelAnimationGroup* slideOutGroup = new QParallelAnimationGroup(this);
	QPropertyAnimation* slideOutAnim = new QPropertyAnimation(firstMsg, "pos");
	QPropertyAnimation* opacityAnim = new QPropertyAnimation(firstMsg, "customOpacity");

	slideOutAnim->setDuration(animDuration - 150);
	slideOutAnim->setEasingCurve(QEasingCurve::InOutSine);
	slideOutAnim->setStartValue(firstMsg->pos());
	slideOutAnim->setEndValue(QPoint(firstMsg->x(), firstMsg->y() - 10));

	opacityAnim->setDuration(animDuration - 150);
	opacityAnim->setEasingCurve(QEasingCurve::InOutSine);
	opacityAnim->setStartValue(1.0);
	opacityAnim->setEndValue(0.0);

	slideOutGroup->addAnimation(slideOutAnim);
	slideOutGroup->addAnimation(opacityAnim);

	// 首个滑出和整体上移同时执行
	int baseY = m_offsetY + spacingY;
	int heightWithSpacing = msgHeight + 10;
	for (int i = 1; i < m_messages.count(); ++i)
	{
		AntMessage* msg = m_messages[i];
		int x = (DesignSystem::instance()->getMainWindow()->width() - msg->width()) / 2;
		QPropertyAnimation* anim = new QPropertyAnimation(msg, "pos");
		anim->setDuration(animDuration - 100);
		anim->setEasingCurve(QEasingCurve::InOutSine);
		anim->setStartValue(QPoint(x, msg->pos().y()));
		anim->setEndValue(QPoint(x, baseY + (i - 1) * heightWithSpacing));
		slideOutGroup->addAnimation(anim);
	}

	// 动画结束后，消息销毁和队列更新
	connect(slideOutAnim, &QPropertyAnimation::finished, firstMsg, &QWidget::deleteLater);

	connect(slideOutGroup, &QParallelAnimationGroup::finished, this, [this]()
		{
			m_messages.removeFirst(); // 移除退出消息
			isAnimating = false;
			m_isBatchAnimating = false;

			if (!m_messages.isEmpty())
			{
				AntMessage* newFirst = m_messages.first();

				// 检查新首消息是否已经计时结束
				if (newFirst->hasTimeoutOccurred())
				{
					// 退出动画
					startExitAnimation();
				}
				// 否则等待其自然计时结束，再发退出请求
			}
		});

	m_isBatchAnimating = true;
	slideOutGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void AntMessageManager::onMessageExitFinished(AntMessage* msg)
{
	// 这里可以做额外清理或通知
}