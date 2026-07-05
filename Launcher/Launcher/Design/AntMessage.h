#pragma once

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainterPath>

class AntMessage : public QWidget
{
	Q_OBJECT
		Q_PROPERTY(qreal customOpacity READ getCustomOpacity WRITE setCustomOpacity)
public:
	enum Type { Info, Success, Error, Warning };

	explicit AntMessage(QWidget* parent, Type type, const QString& message);
	~AntMessage();

	// 启动显示持续时间计时器
	void startDisplayTimer(int ms);

	qreal getCustomOpacity();
	void setCustomOpacity(qreal opacity);

	void setIsExit(bool isExit);
	bool isExit();
	// 是否已经出现超时
	bool hasTimeoutOccurred();

	// 设置进入动画开始结束位置
	void setStartEndPos(QPoint startPos, QPoint endPos)
	{
		m_startPos = startPos;
		m_endPos = endPos;
	}
	// 进入动画开始位置
	QPoint getStartPos() const { return m_startPos; }
	// 进入结束开始位置
	QPoint getEndPos() const { return m_endPos; }
signals:
	// 请求退出
	void requestExit(AntMessage* msg);
	// 退出动画结束信号
	void exitAnimationFinished(AntMessage* msg);
protected:
	void paintEvent(QPaintEvent* event) override;
private slots:
	void onTimeout();
private:
	void initResources();
	void setupTimer();

	bool m_isExit;
	bool m_timeoutOccurred;
	QPoint m_startPos;
	QPoint m_endPos;
	Type m_type;
	QString m_message;
	QString m_svgPath;
	qreal m_customOpacity;

	QTimer* timer;
	QPropertyAnimation* m_posAnim;
	QPropertyAnimation* m_opacityAnim;
	QParallelAnimationGroup* m_animGroup;
};
