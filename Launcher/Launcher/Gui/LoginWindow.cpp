#include "LoginWindow.h"

bool m_autoLogin = true;

LoginWindow::LoginWindow(QWidget* parent)
	: QWidget(parent)
{
	this->setObjectName("LoginWindow");
	ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setFixedSize(720, 400);
	DesignSystem::instance()->setMainWindow(this);// 获取登陆窗口指针

	// 透明容器
	QWidget* contentWidget = new QWidget(this);
	contentWidget->setGeometry(this->rect());
	contentWidget->setAttribute(Qt::WA_TranslucentBackground);
	contentWidget->raise();

	// 整体布局
	QHBoxLayout* mainLayout = new QHBoxLayout(contentWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// 创建左右区域
	QWidget* leftPanel = new QWidget(contentWidget);
	QWidget* rightPanel = new QWidget(contentWidget);
	rightPanel->setStyleSheet("background:white;");
	rightPanel->setMaximumWidth(260);

	mainLayout->addWidget(leftPanel, 6);
	mainLayout->addWidget(rightPanel, 4);

	// 左侧图标
	QLabel* leftImage = new QLabel(leftPanel);
	leftImage->setScaledContents(true);
	leftImage->setAttribute(Qt::WA_TransparentForMouseEvents);

	QPixmap bgPixmap(":/Launcher/Imgs/Login.png");
	if (!bgPixmap.isNull()) {
		leftImage->setPixmap(bgPixmap);
	}

	QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
	leftLayout->setContentsMargins(0, 0, 0, 0);
	leftLayout->addWidget(leftImage);

	// 右侧：登录表单区域
	QVBoxLayout* loginLayout = new QVBoxLayout(rightPanel);
	loginLayout->setContentsMargins(15, 60, 15, 15);
	loginLayout->setSpacing(8);

	//关闭按钮
	CustomToolButton* closeBtn = new CustomToolButton(rightPanel);
	closeBtn->setFixedSize(37, 32);
	closeBtn->setIconSize(QSize(10, 10));
	closeBtn->setIconPixmap(QPixmap(":/Launcher/Imgs/ShutDown.png"));
	closeBtn->setBackgroundColors(Qt::transparent, QColor(220, 20, 60, 150), QColor(220, 20, 60));
	closeBtn->move(210, 3);
	closeBtn->raise();

	// 版本号
	QLabel* versionLabel = new QLabel(rightPanel);
	versionLabel->setText(QString::fromStdString(currentversion));
	versionLabel->setFixedWidth(200);
	versionLabel->setStyleSheet("color: black; font-size: 12px; background: transparent;");
	versionLabel->move(10, 2);
	versionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	versionLabel->raise();

	// 头像
	QPixmap rawAvatar(":/Launcher/Imgs/UserAvatar.png");
	QPixmap roundAvatar = Globals::getRoundedPixmap(rawAvatar);

	QLabel* avatarLabel = new QLabel(rightPanel);
	avatarLabel->setScaledContents(true);
	avatarLabel->setFixedSize(80, 80);
	avatarLabel->setPixmap(roundAvatar);

	// 用户名输入框
	CustomLineEdit* userEdit = new CustomLineEdit(rightPanel);
	userEdit->setPlaceholderText("请输入用户名");
	userEdit->setFixedSize(220, 35);

	// 密码输入框
	CustomLineEdit* pwdEdit = new CustomLineEdit(rightPanel);
	pwdEdit->setPlaceholderText("请输入密码");
	pwdEdit->setEchoMode(QLineEdit::Password);  // 密码框
	pwdEdit->setFixedSize(220, 35);

	// 限制输入：英文 + 数字
	QRegularExpression regExp("^[A-Za-z0-9]*$");
	pwdEdit->setValidator(new QRegularExpressionValidator(regExp, pwdEdit));

	// 水平布局容器
	QWidget* checkContainer = new QWidget(rightPanel);
	QHBoxLayout* checkLayout = new QHBoxLayout(checkContainer);
	checkLayout->setContentsMargins(0, 0, 0, 0);
	checkLayout->setSpacing(50);

	// 选择框
	CustomCheckBox* RememberpwdCheck = new CustomCheckBox("记住密码", rightPanel);
	RememberpwdCheck->setFixedSize(100, 22);
	CustomCheckBox* AutomaticloginCheck = new CustomCheckBox("自动登录", rightPanel);
	AutomaticloginCheck->setFixedSize(100, 22);

	checkLayout->addWidget(RememberpwdCheck);
	checkLayout->addWidget(AutomaticloginCheck);
	checkContainer->setFixedWidth(220);

	// 登录按钮
	CustomButton* loginBtn = new CustomButton("登 录", rightPanel);
	loginBtn->setFixedSize(220, 35);

	// 同意服务协议
	CustomCheckBox* Agreementbtn = new CustomCheckBox("已阅读并同意服务协议", rightPanel);
	Agreementbtn->setFixedSize(160, 22);
	Agreementbtn->setChecked(true);

	//创建底部按钮
	QWidget* helpContainer = new QWidget(rightPanel);
	QHBoxLayout* helpLayout = new QHBoxLayout(helpContainer);
	helpLayout->setContentsMargins(0, 0, 0, 0);
	helpLayout->setSpacing(10);

	// 注册和忘记密码
	QPushButton* noticeBtn = new QPushButton("更新公告", rightPanel);
	QPushButton* forgetBtn = new QPushButton("忘记密码", rightPanel);
	QPushButton* registerBtn = new QPushButton("注册账号", rightPanel);

	noticeBtn->setCursor(Qt::PointingHandCursor);
	forgetBtn->setCursor(Qt::PointingHandCursor);
	registerBtn->setCursor(Qt::PointingHandCursor);

	noticeBtn->setStyleSheet(
		"QPushButton{"
		"background: transparent;"
		"border: none;"
		"color: #00BFFF;"
		"font-size:12px;"
		"font-weight: bold;"
		"}"
		"QPushButton:hover{"
		"color:#0078D7;"
		"}"
	);

	forgetBtn->setStyleSheet(
		"QPushButton{"
		"background: transparent;"
		"border: none;"
		"color: #00BFFF;"
		"font-size:12px;"
		"font-weight: bold;"
		"}"
		"QPushButton:hover{"
		"color:#0078D7;"
		"}"
	);

	registerBtn->setStyleSheet(
		"QPushButton{"
		"background: transparent;"
		"border: none;"
		"color: #00BFFF;"
		"font-size:12px;"
		"font-weight: bold;"
		"}"
		"QPushButton:hover{"
		"color:#0078D7;"
		"}"
	);

	helpLayout->addWidget(noticeBtn);
	helpLayout->addWidget(forgetBtn);
	helpLayout->addWidget(registerBtn);

	// 组装右侧布局
	loginLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);
	loginLayout->addSpacing(10);
	loginLayout->addWidget(userEdit, 0, Qt::AlignCenter);
	loginLayout->addWidget(pwdEdit, 0, Qt::AlignCenter);
	loginLayout->addWidget(checkContainer, 0, Qt::AlignCenter);
	loginLayout->addWidget(loginBtn, 0, Qt::AlignCenter);
	loginLayout->addWidget(Agreementbtn, 0, Qt::AlignCenter);
	loginLayout->addWidget(helpContainer, 0, Qt::AlignCenter);
	loginLayout->addStretch();

	//取消焦点
	this->setFocus();

	//读取账号
	if (m_autoLogin) {
		auto d = LocalDataStore::load();
		userEdit->setText(d.username);
		pwdEdit->setText(d.password);
		m_autoLogin = d.autoLogin;

		// 是否自动登录
		if (m_autoLogin) {
			QTimer::singleShot(0, loginBtn, &QAbstractButton::click);
			loginBtn->setChecked(true);
		}
	}

	//按钮监测
	connect(loginBtn, &QPushButton::clicked, this, [this, AutomaticloginCheck, RememberpwdCheck,userEdit, pwdEdit]()
		{
			if (userEdit->text().isEmpty())
			{
				AntMessageManager::instance()->showMessage(AntMessage::Error, "账号不能为空!");
				return;
			}

			if (pwdEdit->text().isEmpty())
			{
				AntMessageManager::instance()->showMessage(AntMessage::Error, "密码不能为空!");
				return;
			}

			if (pwdEdit->text().length() <= 5) {
				AntMessageManager::instance()->showMessage(AntMessage::Error, "密码不得少于5位数!");
				return;
			}

			std::string ret;
			auto Iskey = true;
			if (Iskey) {
				m_selfName = userEdit->text();

				if (!m_autoLogin) {
					int isautolog = 0;
					if (AutomaticloginCheck->isChecked()) isautolog = 1;
					if (!RememberpwdCheck->isChecked()) {
						userEdit->clear();
						pwdEdit->clear();
					}

					LocalDataStore::savePartial(userEdit->text(), pwdEdit->text(), isautolog);
				}

				onLoginClicked();
				userEdit->clear();
				pwdEdit->clear();
			}
			else {
				AntMessageManager::instance()->showMessage(AntMessage::Error, QString::fromLocal8Bit(ret));
			}
		});

	connect(AutomaticloginCheck, &QAbstractButton::clicked, this, [=]() {
		if (!RememberpwdCheck->isChecked()) {
			AntMessageManager::instance()->showMessage(AntMessage::Error, "请先勾选记住密码!");

			QSignalBlocker blocker(AutomaticloginCheck);
			AutomaticloginCheck->setChecked(false);
		}
		});

	connect(closeBtn, &QPushButton::clicked, this, [=]() {
		qApp->quit();
		});//退出

	connect(noticeBtn, &QPushButton::clicked, this, [=]() {
		if (!noticeDialog) {
			noticeDialog = new NoticeDialog(this);
			noticeDialog->setFixedSize(350, 350);
		}

		noticeDialog->raise();
		noticeDialog->showWithAnimation();
		});

	connect(registerBtn, &QPushButton::clicked, this, [=]() {
		if (!registerDialog) {
			registerDialog = new RegisterDialog(this);
			registerDialog->setFixedSize(300, 350);
		}

		registerDialog->raise();
		registerDialog->showWithAnimation();
		});

	connect(forgetBtn, &QPushButton::clicked, this, [=]() {
		AntMessageManager::instance()->showMessage(AntMessage::Info, "功能正在开发中...");
		});

	Globals::setSystemGlassEnabled(this, true);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::onLoginClicked()
{
	// 关闭现在的窗口
	this->close();

	// 打开主界面
	MainWindow* mainWindow = new MainWindow();
	mainWindow->setWindowIcon(QIcon(":/Launcher/Imgs/Logo.ico"));
	mainWindow->setWindowTitle(QStringLiteral("shaoze"));
	mainWindow->show();
}

void LoginWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		m_dragPosition = event->globalPosition() - QPointF(frameGeometry().topLeft());
		event->accept();
	}
}

void LoginWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		move((event->globalPosition() - m_dragPosition).toPoint());
		event->accept();
	}
}

bool LoginWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
	if (eventType == "windows_generic_MSG") {
		MSG* msg = static_cast<MSG*>(message);

		if (msg->message == WM_NCACTIVATE) {
			if (msg->wParam == FALSE) {
				*result = TRUE;
				return true;
			}
		}

		if (msg->message == WM_NCCALCSIZE) {
			if (msg->wParam == TRUE) {
				*result = 0;
				return true;
			}
		}
	}
#endif
	return QWidget::nativeEvent(eventType, message, result);
}