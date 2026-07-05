#include "../Chat/Chat.h"
#include "ChatPage.h"
#include <QVBoxLayout>
#include <QLabel>

QString m_selfName = QStringLiteral("");
bool b_initSocket = false;
std::atomic_bool g_aiBusy{ false };
bool m_isLogin = true;

ChatPage::ChatPage(QWidget* parent)
    : QWidget(parent)
{
	setObjectName("FunctionPage");
	setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

	manager = new QNetworkAccessManager;

	//刷新信息
	updateinformation();

	auto* root = new QVBoxLayout(this);
	root->setContentsMargins(0, 0, 6, 2);
	root->setSpacing(0);

	//用户列表
	chatItems = {
	{":/Launcher/Imgs/Logo.png","官方聊天室","在线人数: 0人", "10:30 AM", false},
	{":/Launcher/Imgs/Shaoze.png","少泽","给你发了一条私密信息","10:31 AM", false},
	{":/Launcher/Imgs/deepseek.png","AI聊天室(测试版)","给你发了一条私密信息","10:31 AM", false},
	};

	m_chatList = new AntChatListView(this);
	m_chatList->setVerticalScrollBar(new SmoothScrollBar(Qt::Vertical, m_chatList));
	m_listModel = m_chatList->createModel(chatItems);
	m_chatList->setModel(m_listModel);
	m_chatList->setMinimumWidth(175);

	m_stack = new QStackedWidget(this);
	m_stack->addWidget(createChatPage("官方聊天室", 0));
	m_stack->addWidget(createChatPage("少泽", 1));
	m_stack->addWidget(createChatPage("AI聊天室", 2));

	// 聊天区 + 发送区
	rightPanel = new QWidget(this);
	rightPanel->setMinimumWidth(650);
	auto* rightLay = new QVBoxLayout(rightPanel);
	rightLay->setContentsMargins(15, 5, 5, 15);
	rightLay->setSpacing(0);

	//右击菜单
	rightPanel->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(rightPanel, &QWidget::customContextMenuRequested, this,
		[this](const QPoint& pos)
		{
			QMenu menu(rightPanel);
			QAction* actRefresh = menu.addAction("刷新");

			connect(actRefresh, &QAction::triggered, this, [this]() {
				int currentIndex = m_stack->currentIndex();
				clearChatUI(currentIndex);
				Chat::ShutdownSocket();
				b_initSocket = false;
				});

			menu.exec(rightPanel->mapToGlobal(pos));
		});

	auto* bottomGap = new QWidget(rightPanel);
	bottomGap->setFixedHeight(40);
	bottomGap->setAttribute(Qt::WA_TranslucentBackground);
	bottomGap->setStyleSheet("background: transparent;");

	// 发送区域布局
	sendBar = new SendBarWidget(rightPanel);
	sendBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sendBar->setFixedHeight(230);
	sendBar->setFixedWidth(742);

	btnExpression = new CustomToolButton(sendBar);
	btnExpression->setFixedSize(40, 40);
	btnExpression->setIconSize(QSize(20, 20));
	btnExpression->setCheckable(true);
	btnExpression->setIconPixmap(QPixmap(":/Launcher/Imgs/Expression.png"));

	CustomToolButton* btnDocument = new CustomToolButton(sendBar);
	btnDocument->setFixedSize(40, 40);
	btnDocument->setIconSize(QSize(20, 20));
	btnDocument->setCheckable(true);
	btnDocument->setIconPixmap(QPixmap(":/Launcher/Imgs/Document.png"));

	//左上角按钮
	sendBar->topLeftLayout()->addWidget(btnExpression);
	sendBar->topLeftLayout()->addWidget(btnDocument);

	CustomButton* btnSend = new CustomButton("发送");
	btnSend->setFixedSize(100, 35);

	//右下角按钮
	sendBar->bottomRightLayout()->addWidget(btnSend);

	rightLay->addWidget(m_stack, 1);
	rightLay->addWidget(sendBar, 0);

	//表情包界面
	if (!m_emojiPanel) {
		m_emojiPanel = new EmojiPanel(this);

		connect(m_emojiPanel, &EmojiPanel::emojiClicked, this,
			[this](const QString& e) {
				auto* input = sendBar->input();
				if (!input) return;
				input->insertPlainText(e);
				input->setFocus();
			});

		connect(m_emojiPanel, &EmojiPanel::panelClosed, this,
			[this]() {
				btnExpression->setChecked(false);
			});
	}

	// 分割线
	splitter = new QSplitter(Qt::Horizontal, this);
	splitter->setHandleWidth(1);
	splitter->addWidget(m_chatList);
	splitter->addWidget(rightPanel);
	splitter->setCollapsible(0, false);
	splitter->setCollapsible(1, false);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setSizes({ m_chatList->width() + 50,rightPanel->width() });

	root->addWidget(splitter, 1);

	// 更细发送区宽度
	connect(splitter, &QSplitter::splitterMoved, this, [this](int pos, int index) {
		if (index == 1) {
			int newWidth = width() - m_chatList->width() - 30;
			sendBar->setFixedWidth(newWidth);
		}
		});

	// 监听发送
	connect(sendBar, &SendBarWidget::sendRequested, this,
		[this](const QString& text) {
			if (!m_isLogin) {
				AntMessageManager::instance()->showMessage(AntMessage::Info, "请登录账号后重试!", 50);
				return;
			}

			const QString t = text.trimmed();
			if (t.isEmpty()) return;

			QByteArray bytes = t.toUtf8();

			const int chatIndex = m_stack->currentIndex();
			switch (chatIndex)
			{
			case 0:
				::send(g_socket, bytes.constData(), bytes.size(), 0);
				break;
			case 1:
				appendMessage(1, m_selfName, text, m_selfAvatar);
				break;
			case 2:
			{
				//判断UI是否在回复
				if (g_aiBusy.load()) {
					AntMessageManager::instance()->showMessage(AntMessage::Warning, "正在回复中，请稍后再发送!", 50);
					return;
				}

				g_aiBusy.store(true);

				// 显示用户消息
				appendMessage(2, m_selfName, text, m_selfAvatar);
				QPointer<QLabel> aiBubble = appendMessage(2, "DeepSeek(纯文本模型)", "正在思考中......", m_AiAvatar);
				AIchat(text, aiBubble);
				break;
			}
			default:
				break;
			}
		});

	connect(btnSend, &CustomButton::clicked, this, [this] {
		const QString text = sendBar->input()->toPlainText().trimmed();
		if (!text.isEmpty())
			emit sendBar->sendRequested(text);
		sendBar->input()->clear();
		});

	connect(m_chatList, &QListView::clicked, this, [this](const QModelIndex& idx) {
		if (!idx.isValid()) return;
		const int row = idx.row();
		if (row >= 0 && row < m_stack->count())
			m_stack->setCurrentIndex(row);
		});

	connect(btnExpression, &QToolButton::clicked, this, [this]() {
		if (!m_emojiPanel) return;

		if (m_emojiPanel->isVisible()) {
			m_emojiPanel->hide();
			btnExpression->setChecked(false);
		}
		else {
			showEmojiPanel(btnExpression);  // 单独写一个函数计算位置并显示
			btnExpression->setChecked(true);
		}
		});

	if (m_listModel && m_listModel->rowCount() > 0) {
		m_chatList->setCurrentIndex(m_listModel->index(0, 0));
		m_stack->setCurrentIndex(0);
	}

	// 初始信息
	appendMessage(1, "少泽", "我早已麻痹！", m_selfAvatar);
	appendMessage(2, "DeepSeek(纯文本模型)", "你好,我是你的AI助手!", m_AiAvatar);
}

static QString breakLongAsciiRuns(const QString& s, int every = 12)
{
	QString out;
	out.reserve(s.size() * 2);

	int run = 0;
	for (QChar ch : s) {
		const bool isAsciiWord = ch.isLetterOrNumber();
		if (isAsciiWord) {
			run++;
			out += ch;
			if (run >= every) {
				out += QChar(0x200B);
				run = 0;
			}
		}
		else {
			run = 0;
			out += ch;
		}
	}
	return out;
}

static QString makeClickableLinks(const QString& text)
{
	QString t = text.toHtmlEscaped();

	static const QRegularExpression re(
		R"((https?://[^\s<>"']+))",
		QRegularExpression::CaseInsensitiveOption
	);

	t.replace(re, R"(<a href="\1">\1</a>)");
	return t;
}

static bool isNearBottom(QScrollBar* bar)
{
	if (!bar) return true;
	const int slack = qMax(2, bar->pageStep() / 10);
	return (bar->maximum() - bar->value()) <= slack;
}

static int calcBubbleWidthForText(const QFont& font, const QString& shown)
{
	const int maxW = 560;
	const int minW = 30;
	const int leftPad = 12, rightPad = 12;
	const int paddingLR = leftPad + rightPad;

	QFontMetrics fm(font);
	QRect multi = fm.boundingRect(QRect(0, 0, maxW - paddingLR, 100000),
		Qt::TextWordWrap, shown);

	const int oneLine = fm.lineSpacing();
	if (multi.height() > oneLine + 1) {
		int w = multi.width() + paddingLR;
		return qBound(minW, w, maxW);
	}

	int w = fm.horizontalAdvance(shown) + paddingLR;
	return qBound(minW, w, maxW);
}

void ChatPage::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}

bool ChatPage::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		QPoint globalPos = mouseEvent->globalPosition().toPoint();

		// 如果点击的位置不在按钮和面板上，则隐藏面板
		if (m_emojiPanel && m_emojiPanel->isVisible()) {
			if (!m_emojiPanel->geometry().contains(globalPos) &&
				!btnExpression->geometry().contains(btnExpression->mapFromGlobal(globalPos)))
			{
				m_emojiPanel->hide();
				btnExpression->setChecked(false);
			}
		}
	}
	return QObject::eventFilter(obj, event);
}

void ChatPage::showEmojiPanel(CustomToolButton* btnExpression)
{
	m_emojiPanel->adjustSize();
	const QSize panelSize = m_emojiPanel->size();

	const QPoint btnCenterGlobal =
		btnExpression->mapToGlobal(QPoint(btnExpression->width() / 2, btnExpression->height() / 2));

	const QPoint sendBarTopLeftGlobal = sendBar->mapToGlobal(QPoint(0, 0));

	const int gap = 5;
	int x = btnCenterGlobal.x() - panelSize.width() + 445;
	int y = sendBarTopLeftGlobal.y() - panelSize.height() - gap;

	QRect scr = btnExpression->screen()->availableGeometry();
	x = qBound(scr.left() + 6, x, scr.right() - panelSize.width() - 6);
	y = qBound(scr.top() + 6, y, scr.bottom() - panelSize.height() - 6);

	const int arrowCenterX = btnCenterGlobal.x() - x;
	m_emojiPanel->setArrowCenterX(arrowCenterX);

	m_emojiPanel->move(x, y);
	m_emojiPanel->show();
	m_emojiPanel->raise();
	m_emojiPanel->setFocus();
}

void ChatPage::updateinformation() {
	// 刷新信息
	if (!m_gameCheckTimer) {
		m_gameCheckTimer = new QTimer(this);
		m_gameCheckTimer->setInterval(500);

		connect(m_gameCheckTimer, &QTimer::timeout, this, [this]() {
			username = m_selfName;//用户名

			// 连接服务器
			if (!b_initSocket && !m_selfName.trimmed().isEmpty()) {
				if (messages.size() > 0) {
					std::lock_guard<std::mutex> lock(msgMutex);
					messages.clear();
					m_lastMsgIndex = 0;

					clearChatUI(0);
				}

				std::thread(Chat::InitSocket_).detach();
				b_initSocket = true;
			}

			//刷新在线人数
			if (m_listModel) {
				const int userCount = onlineCount.load();
				if (QStandardItem* it = m_listModel->item(0, 0))
					it->setData(QString("在线人数: %1 人").arg(userCount), Qt::UserRole + 2);

				const QModelIndex idx = m_listModel->index(0, 0);
				emit m_listModel->dataChanged(idx, idx);
			}

			// 追加新消息
			std::vector<std::string> local;
			{
				std::lock_guard<std::mutex> lock(msgMutex);
				if (m_lastMsgIndex >= messages.size()) return;
				local.assign(messages.begin() + m_lastMsgIndex, messages.end());
				m_lastMsgIndex = messages.size();
			}

			for (const auto& msg : local) {
				size_t start = msg.find('[');
				size_t sep = msg.find('|', start);
				size_t end = msg.find(']', sep);

				QString senderName;
				if (start != std::string::npos && sep != std::string::npos && end != std::string::npos) {
					senderName = QString::fromUtf8(msg.data() + (sep + 1), int(end - sep - 1));
				}

				QString pureMsg = (end != std::string::npos)
					? QString::fromUtf8(msg.data() + (end + 1), int(msg.size() - (end + 1)))
					: QString::fromUtf8(msg.data(), int(msg.size()));

				appendMessage(0, senderName, pureMsg, m_selfAvatar);
			}
			});
		m_gameCheckTimer->start();
	}
}

void ChatPage::scrollToBottom(int chatIndex)
{
	if (!m_chatUis.contains(chatIndex)) return;
	auto& ui = m_chatUis[chatIndex];
	auto* scroll = ui.scroll;
	if (!scroll) return;

	auto* bar = scroll->verticalScrollBar();
	if (!bar) return;

	ui.programmaticScroll = true;

	QMetaObject::Connection* c = new QMetaObject::Connection;
	*c = connect(bar, &QScrollBar::rangeChanged, scroll, [this, chatIndex, c](int, int) {
		auto& ui2 = m_chatUis[chatIndex];
		auto* bar2 = ui2.scroll->verticalScrollBar();
		if (!bar2) return;

		QSignalBlocker blocker(bar2);
		bar2->setValue(bar2->maximum());

		ui2.programmaticScroll = false;

		QObject::disconnect(*c);
		delete c;
		});

	QTimer::singleShot(0, scroll, [this, chatIndex]() {
		auto& ui2 = m_chatUis[chatIndex];
		auto* bar2 = ui2.scroll->verticalScrollBar();
		if (!bar2) return;

		QSignalBlocker blocker(bar2);
		bar2->setValue(bar2->maximum());
		ui2.programmaticScroll = false;
		});
}

void ChatPage::sendCurrentMessage()
{
	if (!m_input) return;

	const QString text = m_input->text().trimmed();
	if (text.isEmpty()) return;

	const int chatIndex = m_stack->currentIndex();
	appendMessage(chatIndex, m_selfName, text, m_selfAvatar);

	m_input->clear();
	m_input->setFocus();
}

QWidget* ChatPage::createChatPage(const QString& title, int chatIndex)
{
	setStyleSheet(StyleSheet::vListViewQss(QColor(220, 220, 220)));

	auto* page = new QWidget(this);
	auto* rootLay = new QVBoxLayout(page);
	rootLay->setContentsMargins(0, 0, 0, 0);
	rootLay->setSpacing(5);

	auto* titleLabel = new QLabel(title, page);
	QFont titleLabelfont = titleLabel->font();
	titleLabelfont.setPointSize(13);
	titleLabelfont.setBold(true);
	titleLabel->setFont(titleLabelfont);
	titleLabel->setStyleSheet("font-weight:bold;");
	rootLay->addWidget(titleLabel);

	ChatUi ui;
	ui.scroll = new QScrollArea(page);
	SmoothScrollBar* vBar = new SmoothScrollBar(Qt::Vertical, ui.scroll);
	ui.scroll->setVerticalScrollBar(vBar);
	ui.scroll->setWidgetResizable(true);
	ui.scroll->setFrameShape(QFrame::NoFrame);
	ui.scroll->setStyleSheet("background: transparent;");
	ui.scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.scroll->setObjectName("ChatScrollArea");
	ui.scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	auto* bar = ui.scroll->verticalScrollBar();
	connect(bar, &QScrollBar::valueChanged, page, [this, chatIndex](int) {
		auto& ui = m_chatUis[chatIndex];
		if (ui.programmaticScroll) return;
		ui.stickToBottom = isNearBottom(ui.scroll->verticalScrollBar());
		});

	ui.container = new QWidget(ui.scroll);
	ui.msgLay = new QVBoxLayout(ui.container);
	ui.msgLay->setContentsMargins(0, 0, 0, 0);
	ui.msgLay->setSpacing(8);
	ui.msgLay->setAlignment(Qt::AlignTop);

	ui.scroll->setWidget(ui.container);

	rootLay->addWidget(ui.scroll, 1);

	m_chatUis.insert(chatIndex, ui);
	return page;
}

QLabel* ChatPage::appendMessage(int chatIndex, const QString& userName, const QString& content, const QString& avatarPath)
{
	if (!m_chatUis.contains(chatIndex)) return nullptr;
	auto& ui = m_chatUis[chatIndex];
	if (!ui.msgLay) return nullptr;

	const bool shouldFollow = ui.stickToBottom;
	QVBoxLayout* msgLayout = ui.msgLay;

	QWidget* row = new QWidget;
	row->setAttribute(Qt::WA_TranslucentBackground);
	row->setStyleSheet("background: transparent;");

	QHBoxLayout* rowLay = new QHBoxLayout(row);
	rowLay->setContentsMargins(5, 8, 0, 8);
	rowLay->setSpacing(10);

	// 系统消息
	static const QString kServerPrefix = QStringLiteral("[SERVER_MESSAGE]");
	if (content.startsWith(kServerPrefix)) {
		const QString showText = content.mid(kServerPrefix.size()).trimmed();

		rowLay->setContentsMargins(0, 8, 0, 8);
		rowLay->setSpacing(10);

		QLabel* sys = new QLabel(showText, row);
		sys->setWordWrap(true);
		sys->setAlignment(Qt::AlignCenter);
		sys->setTextInteractionFlags(Qt::TextSelectableByMouse);
		sys->setMaximumWidth(510);
		sys->setContentsMargins(5, 3, 5, 4);
		sys->setStyleSheet("background: rgba(255,255,255,255);""color: #444;""border-radius: 12px;""font-size: 15px;");

		rowLay->addStretch();
		rowLay->addWidget(sys);
		rowLay->addStretch();

		msgLayout->addWidget(row);
		if (shouldFollow) scrollToBottom(chatIndex);
		return nullptr;
	}

	// 用户消息
	const bool isSelf = (userName == m_selfName);

	QLabel* avatar = new QLabel(row);
	avatar->setFixedSize(45, 45);
	avatar->setAttribute(Qt::WA_TranslucentBackground);
	avatar->setScaledContents(true);
	avatar->setPixmap(Globals::getRoundedPixmap(avatarPath));

	QWidget* wrapper = new QWidget(row);
	wrapper->setAttribute(Qt::WA_TranslucentBackground);
	wrapper->setStyleSheet("background: transparent;");
	wrapper->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	QVBoxLayout* wrapperLay = new QVBoxLayout(wrapper);
	wrapperLay->setContentsMargins(0, 0, 0, 0);
	wrapperLay->setSpacing(5);

	QLabel* nameLabel = new QLabel(userName, wrapper);
	nameLabel->setStyleSheet("font-size:12px;color:#888;");

	QString fixed = breakLongAsciiRuns(content, 12);

	QLabel* bubble = new QLabel(wrapper);
	bubble->setObjectName("MsgBubble");
	bubble->setTextFormat(Qt::RichText);
	bubble->setWordWrap(true);

	QString rich = makeClickableLinks(fixed);
	bubble->setText(rich);

	bubble->setOpenExternalLinks(true);
	bubble->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	bubble->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
	bubble->setContentsMargins(12, 8, 12, 8);
	bubble->setMaximumWidth(560);
	bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	bubble->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(bubble, &QWidget::customContextMenuRequested, this,
		[this, bubble](const QPoint& pos)
		{
			QMenu menu(nullptr);

			QAction* actCopy = menu.addAction("复制");
			QAction* actCopyAll = menu.addAction("复制全部");

			connect(actCopy, &QAction::triggered, bubble, [bubble]() {
				QGuiApplication::clipboard()->setText(bubble->selectedText());
				});

			connect(actCopyAll, &QAction::triggered, bubble, [bubble]() {
				QGuiApplication::clipboard()->setText(bubble->text());
				});

			menu.exec(bubble->mapToGlobal(pos));
		});
	bubble->setFixedWidth(calcBubbleWidthForText(bubble->font(), fixed));

	if (isSelf) {
		bubble->setStyleSheet("background:pink;color:#000;border-radius:13px;");
		nameLabel->setAlignment(Qt::AlignRight);
	}
	else {
		bubble->setStyleSheet("background:#f2f2f2;color:#000;border-radius:13px;");
		nameLabel->setAlignment(Qt::AlignLeft);
	}

	if (isSelf) {
		wrapperLay->addWidget(nameLabel, 0, Qt::AlignRight);
		wrapperLay->addWidget(bubble, 0, Qt::AlignRight);
	}
	else {
		wrapperLay->addWidget(nameLabel, 0, Qt::AlignLeft);
		wrapperLay->addWidget(bubble, 0, Qt::AlignLeft);
	}

	if (isSelf) {
		rowLay->addStretch();
		rowLay->addWidget(wrapper, 0, Qt::AlignTop);
		rowLay->addWidget(avatar, 0, Qt::AlignTop);
		rowLay->addSpacing(5);
	}
	else {
		rowLay->addWidget(avatar, 0, Qt::AlignTop);
		rowLay->addWidget(wrapper, 0, Qt::AlignTop);
		rowLay->addStretch();
	}

	msgLayout->addWidget(row);
	if (shouldFollow) scrollToBottom(chatIndex);

	return bubble;
}

void ChatPage::clearChatUI(int chatIndex)
{
	if (!m_chatUis.contains(chatIndex)) return;

	auto& ui = m_chatUis[chatIndex];
	QVBoxLayout* lay = ui.msgLay;
	if (!lay) return;

	while (QLayoutItem* item = lay->takeAt(0)) {
		if (QWidget* w = item->widget())
			w->deleteLater();
		delete item;
	}

	ui.stickToBottom = true;
	ui.programmaticScroll = false;
}

void ChatPage::AIchat(const QString& text, QPointer<QLabel> bubble)
{
	if (!bubble || text.isEmpty()) return;

	QNetworkRequest request;
	request.setUrl(QUrl("https://api.deepseek.com/chat/completions"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Accept", "application/json");
	request.setRawHeader("Authorization", "填写密钥");

	QJsonArray messages;

	QJsonObject systemMsg;
	systemMsg["role"] = "system";
	systemMsg["content"] = "You are a helpful assistant";

	QJsonObject userMsg;
	userMsg["role"] = "user";
	userMsg["content"] = text;

	messages.append(systemMsg);
	messages.append(userMsg);

	QJsonObject requestBody;
	requestBody["messages"] = messages;
	requestBody["model"] = "deepseek-chat";
	requestBody["max_tokens"] = 2048;
	requestBody["stream"] = true;
	requestBody["temperature"] = 1;

	//发送请求
	QNetworkReply* reply = manager->post(request, QJsonDocument(requestBody).toJson());

	//处理数据
	QString* full = new QString;

	connect(reply, &QNetworkReply::readyRead, this, [=] {
		while (reply->canReadLine())
		{
			QString line = reply->readLine().trimmed();
			if (!line.startsWith("data: ")) continue;

			line.remove(0, 6);
			if (line == "[DONE]") continue;

			QJsonParseError error;
			QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
			if (error.error != QJsonParseError::NoError) continue;

			QString content = doc.object()["choices"].toArray().first()
				.toObject()["delta"].toObject()["content"].toString();

			if (!content.isEmpty() && bubble)
			{
				(*full) += content;

				QString fixed = breakLongAsciiRuns(*full, 12);
				QString html = makeClickableLinks(fixed);
				html.replace("\r\n", "\n");
				html.replace("\n", "<br/>");

				bubble->setText(html);
				bubble->setFixedWidth(calcBubbleWidthForText(bubble->font(), fixed));

				if (m_chatUis.contains(2) && m_chatUis[2].stickToBottom)scrollToBottom(2);
			}
		}
		});

	connect(reply, &QNetworkReply::finished, this, [=] {
		g_aiBusy.store(false);
		delete full;
		reply->deleteLater();
		});

	connect(reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError) {
		g_aiBusy.store(false);
		});
}