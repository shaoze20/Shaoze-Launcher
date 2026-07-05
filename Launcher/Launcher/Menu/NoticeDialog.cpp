#include "NoticeDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

NoticeDialog::NoticeDialog(QWidget* parent)
	: QWidget(parent)
{
	setObjectName("RegisterPage");

	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet(
		"#RegisterPage {"
		"background: white;"
		"border-radius: 15px;"
		"}"
	);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(20, 20, 20, 20);
	layout->setSpacing(15);

	//公告
	QLabel* NoticeLabel = new QLabel(this);
	NoticeLabel->setText("公告");
	NoticeLabel->setStyleSheet("color: black; font-size: 25px; background: transparent;");
	NoticeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	NoticeLabel->setFixedHeight(40);

	// 更新内容
	QLabel* contentLabel = new QLabel(this);
	contentLabel->setText(
		"2026年4月26日更新日志：\n"
		"1.优化启动器.\n"
		"2.修复已知BUG.\n"
	);
	contentLabel->setStyleSheet("color: black; font-size: 15px; background: transparent;");
	contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

	// 确定
	CustomButton* backBtn = new CustomButton("确定");
	backBtn->setFixedSize(100, 35);

	layout->addWidget(NoticeLabel, 0, Qt::AlignHCenter);
	layout->addWidget(contentLabel);
	layout->addStretch();
	layout->addWidget(backBtn, 0, Qt::AlignHCenter);

	// 位置动画
	posAnim = new QPropertyAnimation(this, "pos");
	posAnim->setDuration(450);
	posAnim->setEasingCurve(QEasingCurve::OutCubic); // 平滑惯性

	mask = new QWidget(parent);
	mask->setStyleSheet("background: rgba(0,0,0,120);");
	mask->setGeometry(parent->rect());
	mask->hide();
	mask->raise();

	maskOpacity = new QGraphicsOpacityEffect(mask);
	mask->setGraphicsEffect(maskOpacity);
	maskOpacity->setOpacity(0);

	maskAnim = new QPropertyAnimation(maskOpacity, "opacity");
	maskAnim->setDuration(posAnim->duration());
	maskAnim->setEasingCurve(posAnim->easingCurve());

	// 动画组同步
	animGroup = new QParallelAnimationGroup(this);
	animGroup->addAnimation(posAnim);
	animGroup->addAnimation(maskAnim);

	connect(backBtn, &QPushButton::clicked, this, [=]() {
		emit backToLogin();
		hideWithAnimation();
		});

	connect(animGroup, &QParallelAnimationGroup::finished, this, [this]() {
		if (posAnim->endValue().toPoint().y() < 0) hide();
		if (maskAnim->endValue().toDouble() == 0.0) mask->hide();
		});

	// 记录动画开始时间
	animTimer = new QElapsedTimer();
	QTimer* smoothTimer = new QTimer(this);

	connect(smoothTimer, &QTimer::timeout, [this]() {
		if (animGroup->state() == QAbstractAnimation::Running) {
			qint64 elapsed = animTimer->elapsed();
			for (int i = 0; i < animGroup->animationCount(); ++i) {
				QAbstractAnimation* anim = animGroup->animationAt(i);
				if (anim->state() == QAbstractAnimation::Running) {
					anim->setCurrentTime(static_cast<int>(elapsed));
				}
			}
		}
		});
	smoothTimer->start(5);
	animTimer->restart();
}

void NoticeDialog::showWithAnimation()
{
	if (width() == 0 || height() == 0) adjustSize();

	QWidget* p = parentWidget();
	int centerX = (p->width() - width()) / 2;
	int centerY = (p->height() - height()) / 2;

	move(centerX, -height());

	posAnim->setStartValue(QPoint(centerX, -height()));
	posAnim->setEndValue(QPoint(centerX, centerY));

	mask->setGeometry(p->rect());
	mask->show();
	mask->raise();
	raise();

	maskAnim->setStartValue(0.0);
	maskAnim->setEndValue(1.0);

	animTimer->restart();

	show();
	animGroup->start();
}

void NoticeDialog::hideWithAnimation()
{
	QWidget* p = parentWidget();
	int centerX = (p->width() - width()) / 2;
	int centerY = (p->height() - height()) / 2;

	posAnim->setStartValue(QPoint(centerX, centerY));
	posAnim->setEndValue(QPoint(centerX, -height()));

	maskAnim->setStartValue(1.0);
	maskAnim->setEndValue(0.0);

	animTimer->restart();

	animGroup->start();
}