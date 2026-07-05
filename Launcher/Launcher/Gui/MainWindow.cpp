#include "../Chat/Chat.h"
#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	QApplication::setQuitOnLastWindowClosed(false);
	this->setObjectName("MainWindow");
	ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setFixedSize(1065, 665);
	DesignSystem::instance()->setMainWindow(this);// 获取主窗口指针
	initTrayIcon(); // 初始化托盘

	// 标题栏
	titleBar = new QWidget(this);
	titleBar->setObjectName("titleBar");
	titleBar->setFixedHeight(45);

	QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
	titleLayout->setContentsMargins(15, 3, 3, 3);
	titleLayout->setAlignment(Qt::AlignVCenter);
	titleLayout->setSpacing(2);

	// logo
	QLabel* logo = new QLabel(titleBar);
	logo->setFixedSize(36, 36);
	logo->setScaledContents(true);
	logo->setPixmap(QPixmap(":/Launcher/Imgs/Logo(Transparent).png"));

	// 标题
	QLabel* titleLabel = new QLabel("Shaoze Launcher", titleBar);
	titleLabel->setStyleSheet("color: black; font-size: 13px; font-weight: bold;");
	titleLabel->setAlignment(Qt::AlignCenter);

	// 头像
	QPixmap rawAvatar(":/Launcher/Imgs/UserAvatar.png");
	QPixmap roundAvatar = Globals::getRoundedPixmap(rawAvatar);
	QLabel* Avatar = new QLabel(titleBar);
	Avatar->setFixedSize(34, 34);
	Avatar->setScaledContents(true);
	Avatar->setPixmap(roundAvatar);

	// 用户名
	QLabel* UserNameLabel = new QLabel("  " + m_selfName + "     ", titleBar);
	UserNameLabel->setStyleSheet("color: black; font-size: 13px;");
	UserNameLabel->setAlignment(Qt::AlignCenter);

	// 最小化
	CustomToolButton* btnMin = new CustomToolButton(titleBar);
	btnMin->setIconPixmap(QPixmap(":/Launcher/Imgs/Minimize_.png"));
	btnMin->setDrawBackground(true);
	btnMin->setFixedSize(45, 40);
	btnMin->setIconSize(QSize(12, 12));
	btnMin->raise();

	// 关闭按钮
	CustomToolButton* btnClose = new CustomToolButton(titleBar);
	btnClose->setIconPixmap(QPixmap(":/Launcher/Imgs/ShutDown.png"));
	btnClose->setBackgroundColors(Qt::transparent, QColor(220, 20, 60, 150), QColor(220, 20, 60));
	btnClose->setDrawBackground(true);
	btnClose->setFixedSize(45, 40);
	btnClose->setIconSize(QSize(12, 12));
	btnClose->raise();

	// 标题栏主布局
	titleLayout->addWidget(logo);
	titleLayout->addSpacing(10);
	titleLayout->addWidget(titleLabel);
	titleLayout->addStretch();
	titleLayout->addWidget(Avatar);
	titleLayout->addWidget(UserNameLabel);
	titleLayout->addWidget(btnMin);
	titleLayout->addWidget(btnClose);

	// 内容背景
	m_container = new QWidget(this);
	m_container->setObjectName("mainContainer");
	m_container->setFixedSize(width() - 10, height() - 50);

	// 左区域
	m_leftPanel = new QWidget(m_container);
	m_leftPanel->setObjectName("leftPanel");
	m_leftPanel->setFixedWidth(60);

	// 垂直布局
	QVBoxLayout* leftLayout = new QVBoxLayout(m_leftPanel);
	leftLayout->setContentsMargins(0, 10, 0, 5);
	leftLayout->setSpacing(2);
	leftLayout->setAlignment(Qt::AlignHCenter);

	// 创建按钮
	m_btnGroup = new QButtonGroup(this);
	m_btnGroup->setExclusive(true);

	QStringList icons = {
		":/Launcher/Imgs/Home.png",
		":/Launcher/Imgs/Panel.png",
		":/Launcher/Imgs/Hotspot.png",
		":/Launcher/Imgs/Chat.png",
		":/Launcher/Imgs/Game.png",
	};

	QStringList tooltips = {
		"主页",
		"硬件信息",
		"热门网站",
		"聊天室",
		"游戏",
	};

	int index = 0;
	for (const QString& path : icons) {
		CustomGlowButton* btn = new CustomGlowButton(m_leftPanel);
		btn->setIconPixmap(QPixmap(path));
		btn->setFixedSize(47, 47);
		btn->setIconSize(QSize(30, 30));
		btn->setCheckable(true);
		btn->setTooltipText(tooltips[index]);

		m_btnGroup->addButton(btn, index);
		leftLayout->addWidget(btn, 0, Qt::AlignHCenter);

		index++;
	}

	leftLayout->addStretch();

	// 设置
	CustomGlowButton* settingBtn = new CustomGlowButton(m_leftPanel);
	settingBtn->setIconPixmap(QPixmap(":/Launcher/Imgs/settings.png"));
	settingBtn->setFixedSize(47, 47);
	settingBtn->setIconSize(QSize(30, 30));
	settingBtn->setCheckable(true);
	settingBtn->setTooltipText("设置");
	m_btnGroup->addButton(settingBtn, index);
	leftLayout->addWidget(settingBtn, 0, Qt::AlignHCenter);

	// 指示条
	m_indicator = new QWidget(m_leftPanel);
	m_indicator->setFixedWidth(4);
	m_indicator->setStyleSheet("background-color: #1F51FF; border-radius: 2px;");
	m_indicator->raise();
	QWidget* firstBtn = m_btnGroup->button(0);
	if (firstBtn) {
		m_indicator->move(0, firstBtn->y());
		m_indicator->setFixedHeight(firstBtn->height());
	}

	// 右侧区域
	m_stack = new SlideStackedWidget(m_container);
	m_stack->setObjectName("rightPanel");

	// 创建页面
	HomePage* homePage = new HomePage();
	HardwarePage* hardwarePage = new HardwarePage();
	HotspotPage* hotspotPage = new HotspotPage();
	ChatPage* chatPage = new ChatPage();
	GamePage* gamePage = new GamePage();
	SettingPage* settingPage = new SettingPage();

	m_stack->addWidget(homePage);
	m_stack->addWidget(hardwarePage);
	m_stack->addWidget(hotspotPage);
	m_stack->addWidget(chatPage);
	m_stack->addWidget(gamePage);
	m_stack->addWidget(settingPage);

	QHBoxLayout* layout = new QHBoxLayout(m_container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_leftPanel);
	layout->addWidget(m_stack);

	// 样式
	this->setStyleSheet(R"(
        QWidget#mainContainer {
            background: transparent;
        }

        QWidget#leftPanel {
            background: transparent;
        }

        QWidget#rightPanel {
            background: transparent;
        }
    )");

	// 初始居中
	m_container->move(
		(width() - m_container->width()) / 2,
		65 + (height() - 65 - m_container->height()) / 2
	);

	// 页面切换
	connect(m_btnGroup, &QButtonGroup::idClicked, this, [this](int id) {
		if (m_stack->currentIndex() == id && !m_animating) return;

		if (m_animating) {
			m_pendingPageId = id;
			return;
		}

		auto startSwitch = [this](int targetId) {
			m_animating = true;
			m_pendingPageId = -1;

			QWidget* nextPage = m_stack->widget(targetId);
			QAbstractButton* btn = qobject_cast<QAbstractButton*>(m_btnGroup->button(targetId));
			if (!btn) {
				m_animating = false;
				return;
			}

			for (auto b : m_btnGroup->buttons()) {
				if (auto ab = qobject_cast<QAbstractButton*>(b)) {
					ab->setChecked(ab == btn);
				}
			}

			if (m_indicator) {
				QRect targetRect(0, btn->y() + (btn->height() - btn->height() * 0.5) / 2, 4, btn->height() * 0.5);

				QPropertyAnimation* anim = new QPropertyAnimation(m_indicator, "geometry", this);
				anim->setDuration(250);
				anim->setStartValue(m_indicator->geometry());
				anim->setEndValue(targetRect);
				anim->setEasingCurve(QEasingCurve::InOutCubic);

				connect(anim, &QPropertyAnimation::valueChanged, this, [this]() {
					m_leftPanel->update();
					});

				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}

			m_stack->slideFromBottomToTop(nextPage, 250, [this]() {
				m_stack->update();
				m_stack->repaint();
				m_animating = false;

				if (m_pendingPageId != -1 && m_pendingPageId != m_stack->currentIndex()) {
					int nextId = m_pendingPageId;
					m_pendingPageId = -1;

					QMetaObject::invokeMethod(this, [this, nextId]() {
						QAbstractButton* b = m_btnGroup->button(nextId);
						if (b) b->click();
						}, Qt::QueuedConnection);
				}
				});
			};

		startSwitch(id);
		});

	// 默认选中
	m_btnGroup->button(0)->setChecked(true);
	m_stack->setCurrentIndex(0);

	// 按钮监视
	connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
	connect(btnClose, &QPushButton::clicked, this, [=]() {
		if (!m_closeMinimizeCard->isChecked()) {
			Chat::ShutdownSocket();
			QApplication::quit();
		}
		else {
			this->hide();
		}
		});

	Globals::setSystemGlassEnabled(this, true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	if (!m_container || !m_leftPanel || !m_stack)
		return;

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);
	p.setPen(Qt::NoPen);

	const qreal radius = 8.0;

	QRectF containerRect = QRectF(m_container->geometry());
	QRectF leftRect = QRectF(m_leftPanel->geometry()).translated(m_container->pos());

	// 整体白色圆角背景
	QPainterPath containerPath;
	containerPath.addRoundedRect(containerRect, radius, radius);
	p.setClipPath(containerPath);
	p.fillPath(containerPath, DesignSystem::instance()->color.background);

	// 左侧灰色区域
	p.save();
	p.setClipPath(containerPath);
	p.fillRect(leftRect, DesignSystem::instance()->color.sidePanel);
	p.restore();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && event->pos().y() <= 45) {
		m_dragging = true;
		m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
	}

	QWidget::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (m_dragging && (event->buttons() & Qt::LeftButton)) {
		move(event->globalPosition().toPoint() - m_dragPos);
	}

	QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	m_dragging = false;
	QWidget::mouseReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	// 保持居中
	if (m_container) {
		m_container->setFixedSize(width() - 8, height() - 48);
		m_container->move((width() - m_container->width()) / 2, 45);
	}

	if (titleBar) {
		titleBar->setGeometry(0, 0, width(), 45);
	}

	// 保持指示条高度与当前按钮一致
	for (auto btn : m_btnGroup->buttons()) {
		if (btn->isChecked() && m_indicator) {
			m_indicator->setFixedHeight(btn->height() * 0.5);
			m_indicator->move(0, btn->y() + 22);
			break;
		}
	}
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
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

// 托盘菜单
void MainWindow::initTrayIcon()
{
	if (m_trayIcon) return;

	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setIcon(QIcon(":/Launcher/Imgs/Logo.ico"));

	m_trayMenu = new QMenu(this);

	QAction* showAction = new QAction("打开", this);
	QAction* quitAction = new QAction("退出", this);

	connect(showAction, &QAction::triggered, this, [this]() {
		this->showNormal();
		this->raise();
		this->activateWindow();
		});

	connect(quitAction, &QAction::triggered, this, [this]() {
		Chat::ShutdownSocket();
		QApplication::quit();
		});

	m_trayMenu->addAction(showAction);
	m_trayMenu->addSeparator();
	m_trayMenu->addAction(quitAction);

	m_trayIcon->setContextMenu(m_trayMenu);

	connect(m_trayIcon, &QSystemTrayIcon::activated, this,
		[this](QSystemTrayIcon::ActivationReason reason) {
			if (reason == QSystemTrayIcon::Trigger ||
				reason == QSystemTrayIcon::DoubleClick) {

				this->showNormal();
				this->raise();
				this->activateWindow();
			}
		});

	m_trayIcon->show();
}