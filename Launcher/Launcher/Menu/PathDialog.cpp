#include "PathDialog.h"

PathDialog::PathDialog(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags(Qt::FramelessWindowHint);

	// 全局拦截层
	m_blockMask = new QWidget(this);
	m_blockMask->setGeometry(rect());
	m_blockMask->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	m_blockMask->setStyleSheet("background: transparent;");

	// 计时器
	m_frameTimer = new QTimer(this);
	m_frameTimer->setTimerType(Qt::PreciseTimer);
	int interval = 1000 / screen()->refreshRate();
	m_frameTimer->setInterval(interval);

	// 弹窗
	m_panel = new QWidget(this);
	m_panel->setFixedSize(450, 250);
	m_panel->setStyleSheet(R"(
        QWidget {
            background-color: #dddddd;
            border-radius: 15px;
        }
    )");

	// 主布局
	QVBoxLayout* mainLayout = new QVBoxLayout(m_panel);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setSpacing(20);

	// 标签
	QLabel* pathLabel = new QLabel("游戏路径:", m_panel);
	pathLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

	// 输入框
	m_pathEdit = new CustomLineEdit(m_panel);
	m_pathEdit->setPlaceholderText("请输入或选择游戏安装目录...");
	m_pathEdit->setFixedSize(380, 40);

	// 横向功能按钮行
	QHBoxLayout* toolBtnLayout = new QHBoxLayout();
	toolBtnLayout->setSpacing(50);

	CustomButton* btnSteam = new CustomButton("Steam路径", m_panel);
	btnSteam->setFontSize(10);
	btnSteam->setTextColor(Qt::black);
	btnSteam->setBackgroundColor(QColor(255, 160, 180));
	btnSteam->setFixedSize(100, 35);
	btnSteam->setRadius(18);
	CustomButton* btnSelect = new CustomButton("选择路径", m_panel);
	btnSelect->setFontSize(10);
	btnSelect->setTextColor(Qt::black);
	btnSelect->setBackgroundColor(QColor(255, 160, 180));
	btnSelect->setFixedSize(100, 35);
	btnSelect->setRadius(18);

	// 添加到布局
	toolBtnLayout->addStretch();
	toolBtnLayout->addWidget(btnSteam);
	toolBtnLayout->addWidget(btnSelect);
	toolBtnLayout->addStretch();

	// 内容区域容器
	QWidget* contentWidget = new QWidget(m_panel);
	contentWidget->setStyleSheet(R"(
        QWidget {
            background-color: #eeeeee;
            border-radius: 10px;
        }
    )");

	// 内容布局
	QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(15, 15, 15, 15);
	contentLayout->setSpacing(15);

	// 放入内容
	contentLayout->addWidget(pathLabel);
	contentLayout->addWidget(m_pathEdit);
	contentLayout->addLayout(toolBtnLayout);

	// 加到主布局
	mainLayout->addWidget(contentWidget);
	mainLayout->addStretch();

	// 底端操作按钮
	QHBoxLayout* bottomLayout = new QHBoxLayout();
	bottomLayout->setSpacing(15);

	btnOk = new CustomButton("保存", m_panel);
	btnOk->setFixedSize(100, 35);
	btnCancel = new CustomButton("取消", m_panel);
	btnCancel->setFixedSize(100, 35);

	bottomLayout->addStretch(); // 将按钮推向右侧
	bottomLayout->addWidget(btnCancel);
	bottomLayout->addWidget(btnOk);

	mainLayout->addLayout(bottomLayout);

	// 层级管理
	m_blockMask->raise();
	m_panel->raise();

	// 监视按钮
	connect(btnOk, &QPushButton::clicked, this, &PathDialog::confirmed);
	connect(btnCancel, &QPushButton::clicked, this, &PathDialog::canceled);

	connect(btnSelect, &QPushButton::clicked, this, [this]()
		{
			QWidget* host = window();
			QFileDialog dlg(host, tr("选择程序"));
			dlg.setFileMode(QFileDialog::ExistingFile);
			dlg.setNameFilter(tr("可执行文件 (*.exe)"));
			dlg.setOption(QFileDialog::DontUseNativeDialog, true);

			QString temp_exePath;
			if (dlg.exec() == QDialog::Accepted) {
				temp_exePath = dlg.selectedFiles().value(0);
			}

			if (!temp_exePath.isEmpty()) {
				m_execPath = temp_exePath;
				m_pathEdit->setText(temp_exePath);
			}
		});

	connect(btnSteam, &QPushButton::clicked, this, [this]()
		{
			m_pathEdit->setText(m_steamPath);
		});

	//高频计时器
	connect(m_frameTimer, &QTimer::timeout, this, [this]() {
		if (!m_animating) return;

		qreal t = qreal(m_elapsedTimer.elapsed()) / qreal(m_animDuration);
		if (t > 1.0) t = 1.0;

		qreal k = 1.0 - qPow(1.0 - t, 3.0);

		if (m_opening) {
			m_progress = k;
		}
		else {
			m_progress = 1.0 - k;
		}

		m_maskOpacity = 100.0 * m_progress;
		m_panelOffsetY = int((1.0 - m_progress) * 35.0);
		m_panelOpacity = m_progress;

		updatePanelLayout();
		update();

		if (t >= 1.0) {
			m_frameTimer->stop();
			m_animating = false;

			if (m_opening) {
				m_useSnapshot = false;
				m_panel->show();
				m_panel->raise();
			}
			else {
				hide();
				m_closing = false;
				m_useSnapshot = false;
				m_panel->hide();
			}
		}
		});
}

void PathDialog::resizeEvent(QResizeEvent*)
{
	if (parentWidget())
		setGeometry(parentWidget()->rect());

	if (m_blockMask)
		m_blockMask->setGeometry(rect());

	updatePanelLayout();
}

void PathDialog::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QColor maskColor(0, 0, 0, int(m_maskOpacity));
	p.setPen(Qt::NoPen);
	p.setBrush(maskColor);
	p.drawRoundedRect(rect(), 10, 10);

	if (m_useSnapshot && !m_panelSnapshot.isNull()) {
		QSize logicalSize = m_panelSnapshot.size() / m_panelSnapshot.devicePixelRatio();

		const int w = logicalSize.width();
		const int h = logicalSize.height();

		QPoint center((width() - w) / 2, (height() - h) / 2 - m_panelOffsetY);
		QRect target(center, QSize(w, h));

		p.setOpacity(m_panelOpacity);

		QPainterPath path;
		path.addRoundedRect(target, 15, 15);

		p.setClipPath(path);
		p.drawPixmap(target, m_panelSnapshot);
		p.setClipping(false);

		p.setOpacity(1.0);
	}
}

void PathDialog::updatePanelLayout()
{
	int w = m_panel->width();
	int h = m_panel->height();

	QPoint center((width() - w) / 2, (height() - h) / 2);
	m_panel->move(center.x(), center.y() - m_panelOffsetY);
}

void PathDialog::showWithAnimation()
{
	if (m_animating) return;

	m_closing = false;
	m_opening = true;
	m_animating = true;

	m_progress = 0.0;
	m_maskOpacity = 0.0;
	m_panelOpacity = 0.0;
	m_panelOffsetY = 35;

	if (parentWidget())
		setGeometry(parentWidget()->rect());

	updatePanelLayout();

	show();
	raise();

	updatePanelLayout();

	m_panel->show();
	m_panel->raise();
	m_panel->update();
	qApp->processEvents();

	m_panelSnapshot = m_panel->grab();
	m_useSnapshot = true;
	m_panel->hide();

	m_elapsedTimer.restart();
	m_frameTimer->start();
}

void PathDialog::fadeOutAndClose()
{
	if (m_closing || m_animating) return;

	m_closing = true;
	m_opening = false;
	m_animating = true;

	if (parentWidget())
		setGeometry(parentWidget()->rect());

	updatePanelLayout();

	m_panel->show();
	m_panel->raise();
	m_panel->update();
	qApp->processEvents();

	m_panelSnapshot = m_panel->grab();
	m_useSnapshot = true;

	m_panel->hide();

	m_elapsedTimer.restart();
	m_frameTimer->start();
}

void PathDialog::setPath(const QString& path, const QString& steampath)
{
	if (m_pathEdit) m_pathEdit->setText(path);
	m_steamPath = steampath;
}

QString PathDialog::getPath() const
{
	if (m_pathEdit)
		return m_pathEdit->text();

	return QString();
}