#include "../Chat/Chat.h"
#include "SettingPage.h"

SettingCard* m_closeMinimizeCard = nullptr;
SettingCard* m_gameMinimizeCard = nullptr;
SettingCard* m_glassCard = nullptr;

SettingPage::SettingPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SettingPage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    CustomTabWidget* tabWidget = new CustomTabWidget(this);

    // 个人信息
    QWidget* pageUser = new QWidget();
    QVBoxLayout* userLayout = new QVBoxLayout(pageUser);
    userLayout->setContentsMargins(10, 0, 0, 0);
    userLayout->setSpacing(20);
    {
        // 头像
        QPixmap rawAvatar(":/Launcher/Imgs/UserAvatar.png");
        QPixmap roundAvatar = Globals::getRoundedPixmap(rawAvatar);
        QLabel* Avatar = new QLabel(pageUser);
        Avatar->setFixedSize(100, 100);
        Avatar->setScaledContents(true);
        Avatar->setPixmap(roundAvatar);

        // 用户名 
        QLabel* usernameLabel = new QLabel(QString("用户名：%1").arg(m_selfName), pageUser);
        usernameLabel->setStyleSheet("font-size: 20px; color: black;font-weight: 700;");

        // 退出按钮
        CustomButton* logoutBtn = new CustomButton("退出登录", pageUser);
        logoutBtn->setFixedSize(120, 40);

        userLayout->addWidget(Avatar);
        userLayout->addWidget(usernameLabel);
        userLayout->addWidget(logoutBtn);
        userLayout->addStretch();

        connect(logoutBtn, &QPushButton::clicked, this, &SettingPage::onLogoutClicked);
    }

    // 基础设置
    QWidget* pageGeneral = new QWidget();
    QVBoxLayout* generalLayout = new QVBoxLayout(pageGeneral);
    generalLayout->setContentsMargins(0, 0, 0, 0);
    {
        m_closeMinimizeCard = new SettingCard(pageGeneral);
        m_closeMinimizeCard->setImage(QPixmap(":/Launcher/Imgs/Minimize.png"));
        m_closeMinimizeCard->setFixedSize(975, 65);
        m_closeMinimizeCard->setDescription("关闭时最小化到托盘");
        m_closeMinimizeCard->setChecked(false);

        m_gameMinimizeCard = new SettingCard(pageGeneral);
        m_gameMinimizeCard->setImage(QPixmap(":/Launcher/Imgs/Minimize.png"));
        m_gameMinimizeCard->setFixedSize(975, 65);
        m_gameMinimizeCard->setDescription("开始游戏自动最小化");
        m_gameMinimizeCard->setChecked(true);

        // 添加布局
        generalLayout->addWidget(m_closeMinimizeCard);
        generalLayout->addWidget(m_gameMinimizeCard);
        generalLayout->addStretch();
    }

    // 主题美化
    QWidget* pageTheme = new QWidget();
    QVBoxLayout* themeLayout = new QVBoxLayout(pageTheme);
    themeLayout->setContentsMargins(0, 0, 0, 0);
    {
        m_glassCard = new SettingCard(pageGeneral);
        m_glassCard->setImage(QPixmap(":/Launcher/Imgs/Windows.png"));
        m_glassCard->setFixedSize(975, 65);
        m_glassCard->setDescription("启用透明效果");
        m_glassCard->setChecked(true);

        // 添加布局
        themeLayout->addWidget(m_glassCard);
        themeLayout->addStretch();

        // 监听控件
        connect(m_glassCard, &SettingCard::toggled, this, [this](bool enabled) {
            const auto windows = QApplication::topLevelWidgets();
            for (QWidget* w : windows) {
                if (!w) continue;
                Globals::setSystemGlassEnabled(w, enabled);
            }
            });
    }

    // 软件反馈
    QWidget* pageAdvanced = new QWidget();
    QVBoxLayout* advancedLayout = new QVBoxLayout(pageAdvanced);
    advancedLayout->setContentsMargins(0, 0, 0, 0);
    {
        SelectableBorderButton* btn1 = new SelectableBorderButton("进不去游戏", this);
        SelectableBorderButton* btn2 = new SelectableBorderButton("软件效果不好", this);
        SelectableBorderButton* btn3 = new SelectableBorderButton("软件体验差", this);
        SelectableBorderButton* btn4 = new SelectableBorderButton("软件卡顿", this);
        SelectableBorderButton* btn5 = new SelectableBorderButton("软件有BUG", this);

        CustomButton* sendBtn = new CustomButton("提交反馈", this);
        sendBtn->setFixedSize(150, 45);

        advancedLayout->addWidget(btn1);
        advancedLayout->addWidget(btn2);
        advancedLayout->addWidget(btn3);
        advancedLayout->addWidget(btn4);
        advancedLayout->addWidget(btn5);
        advancedLayout->addSpacing(20);
        advancedLayout->addWidget(sendBtn,0,Qt::AlignCenter);
        advancedLayout->addStretch();

        connect(sendBtn, &QAbstractButton::clicked, this, [=]() {
            AntMessageManager::instance()->showMessage(AntMessage::Success, "提交反馈成功!", 50);
            });
    }

    // 关于软件
    QWidget* pageAbout = new QWidget();
    QVBoxLayout* aboutLayout = new QVBoxLayout(pageAbout);
    aboutLayout->setContentsMargins(0, 0, 0, 0);
    {
        // 图片
        QLabel* logoLabel = new QLabel();
        QPixmap pix(":/Launcher/Imgs/Logo(Transparent).png");
        logoLabel->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);

        // 版本号
        QLabel* versionLabel = new QLabel(QString(QString::fromStdString(g_version)));
        versionLabel->setAlignment(Qt::AlignCenter);
        versionLabel->setStyleSheet("color: gray; font-size: 20px;");

        // 友情链接
        QLabel* linkTitle = new QLabel("友情链接:");
        linkTitle->setStyleSheet("color: whlie; font-size: 30px;");

        // 左侧横排链接容器
        QWidget* linkWidget = new QWidget();
        QHBoxLayout* linkLayout = new QHBoxLayout(linkWidget);
        linkLayout->setContentsMargins(0, 0, 0, 0);
        linkLayout->setSpacing(12);
        linkLayout->setAlignment(Qt::AlignLeft);

        // 链接
        QFont f;
        f.setPointSize(12);

        LinkLabel* link1 = new LinkLabel("访问官网", "https://github.com/shaoze20/Shaoze-Launcher");
        link1->setFont(f);

        LinkLabel* link2 = new LinkLabel("用户协议", "https://github.com/shaoze20/Shaoze-Launcher");
        link2->setFont(f);

        LinkLabel* link3 = new LinkLabel("隐私政策", "https://github.com/shaoze20/Shaoze-Launcher");
        link3->setFont(f);

        LinkLabel* link4 = new LinkLabel("Discord", "https://discord.gg/9xztfDnC5k");
        link4->setFont(f);

        LinkLabel* link5 = new LinkLabel("QQ交流群", "https://qm.qq.com/q/gBeT8OYjD2");
        link5->setFont(f);

        LinkLabel* link6 = new LinkLabel("Bilibili", "https://b23.tv/7kAaAe6");
        link6->setFont(f);

        LinkLabel* link7 = new LinkLabel("爱发电", "https://afdian.com/a/shaoze");
        link7->setFont(f);

        // 加到横向布局里
        linkLayout->addWidget(link1);
        linkLayout->addWidget(link2);
        linkLayout->addWidget(link3);
        linkLayout->addWidget(link4);
        linkLayout->addWidget(link5);
        linkLayout->addWidget(link6);
        linkLayout->addWidget(link7);
        linkLayout->addStretch();

        aboutLayout->addWidget(logoLabel);
        aboutLayout->addWidget(versionLabel);
        aboutLayout->addSpacing(100);
        aboutLayout->addWidget(linkTitle);
        aboutLayout->addWidget(linkWidget);
        aboutLayout->addStretch();
    }

    // 添加选项卡
    tabWidget->addTab("个人信息", pageUser);
    tabWidget->addTab("基础设置", pageGeneral);
    tabWidget->addTab("主题美化", pageTheme);
    tabWidget->addTab("软件反馈", pageAdvanced);
    tabWidget->addTab("关于软件", pageAbout);

    layout->addWidget(tabWidget);
}

void SettingPage::onLogoutClicked()
{
    m_autoLogin = false;
    b_initSocket = false;
    Chat::ShutdownSocket();

    QWidget* mainWin = window();   // 当前所属主窗口
    mainWin->close();
    mainWin->deleteLater();

    LoginWindow* loginWindow = new LoginWindow();
    loginWindow->setWindowIcon(QIcon(":/Launcher/Imgs/Logo.ico"));
    loginWindow->setWindowTitle(QStringLiteral("shaoze"));
    loginWindow->show();
}