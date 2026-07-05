#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

RegisterDialog::RegisterDialog(QWidget* parent)
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

	//注册界面
	QLabel* RegisterLabel = new QLabel(this);
	RegisterLabel->setText("注册界面");
	RegisterLabel->setStyleSheet("color: black; font-size: 25px; background: transparent;");
	RegisterLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	RegisterLabel->setFixedHeight(40);

	CustomLineEdit* user = new CustomLineEdit(this);
	user->setFixedSize(250, 40);
	user->setPlaceholderText("用户名");

	CustomLineEdit* pwd = new CustomLineEdit(this);
	pwd->setFixedSize(250, 40);
	pwd->setPlaceholderText("密码");
	pwd->setEchoMode(QLineEdit::Password);

	CustomLineEdit* pwd2 = new CustomLineEdit(this);
	pwd2->setFixedSize(250, 40);
	pwd2->setPlaceholderText("确认密码");
	pwd2->setEchoMode(QLineEdit::Password);

	// 限制输入：英文 + 数字
	QRegularExpression regExp("^[A-Za-z0-9]*$");
	pwd->setValidator(new QRegularExpressionValidator(regExp, pwd));
	pwd2->setValidator(new QRegularExpressionValidator(regExp, pwd2));

	CustomButton* registerBtn = new CustomButton("注册账号");
	registerBtn->setFixedSize(250, 40);
	CustomButton* backBtn = new CustomButton("返回登录");
	backBtn->setFixedSize(250, 40);

	layout->addWidget(RegisterLabel, 0, Qt::AlignHCenter);
	layout->addWidget(user, 0, Qt::AlignHCenter);
	layout->addWidget(pwd, 0, Qt::AlignHCenter);
	layout->addWidget(pwd2, 0, Qt::AlignHCenter);
	layout->addWidget(registerBtn, 0, Qt::AlignHCenter);
	layout->addWidget(backBtn, 0, Qt::AlignHCenter);
	layout->addStretch();

	connect(registerBtn, &QPushButton::clicked, this, [this, user, pwd, pwd2]()
		{
			if (user->text().isEmpty())
			{
				AntMessageManager::instance()->showMessage(AntMessage::Error, "账号不能为空!");
				return;
			}

			if (pwd->text().isEmpty())
			{
				AntMessageManager::instance()->showMessage(AntMessage::Error, "密码不能为空!");
				return;
			}

			if (pwd2->text().isEmpty())
			{
				AntMessageManager::instance()->showMessage(AntMessage::Error, "密码不能为空!");
				return;
			}

			if (pwd->text().length() <= 5 || pwd2->text().length() <= 5) {
				AntMessageManager::instance()->showMessage(AntMessage::Error, "密码不得少于5位数!");
				return;
			}

		    std::string ret = "成功";
			if (!ret.empty()) {
				AntMessageManager::instance()->showMessage(AntMessage::Success, QString::fromLocal8Bit(ret));

				user->clear();
				pwd->clear();
				pwd2->clear();
			}
			else {
				AntMessageManager::instance()->showMessage(AntMessage::Error, "注册失败!");
			}
		});

	connect(backBtn, &QPushButton::clicked, this, [=]() {
		emit backToLogin();
		hideWithAnimation();
		});

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

void RegisterDialog::showWithAnimation()
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

void RegisterDialog::hideWithAnimation()
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