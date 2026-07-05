#include "GamePage.h"

GamePage::GamePage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("GamePage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    // 主布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QStackedWidget* m_stack = new QStackedWidget(this);
    m_stack->setObjectName("gameStack");
    m_stack->setStyleSheet(R"(
        QStackedWidget#gameStack {
            background: transparent;
            border: none;
        }
    )");
    layout->addWidget(m_stack);

    // 游戏界面
    TkaPage* m_tkaPage = new TkaPage(this);
    WuwaPage* m_wuwaPage = new WuwaPage(this);
    YsPage* m_ysPage = new YsPage(this);
    PvzPage* m_pvzPage = new PvzPage(this);

    // 主页
    QWidget* homePage = new QWidget(this);
    homePage->setObjectName("gameHomePage");
    homePage->setStyleSheet(R"(
        QWidget#gameHomePage {
            background: transparent;
            border: none;
        }
    )");
    QVBoxLayout* homeLayout = new QVBoxLayout(homePage);
    homeLayout->setContentsMargins(10, 10, 10, 10);
    homeLayout->setSpacing(0);

    // TAB界面
    CustomTabWidget* tabWidget = new CustomTabWidget(homePage);
    homeLayout->addWidget(tabWidget);

    // 二次元区
    QWidget* pageGame = new QWidget();
    QHBoxLayout* gameLayout = new QHBoxLayout(pageGame);
    gameLayout->setSpacing(10);
    gameLayout->setContentsMargins(10, 10, 10, 10);
    gameLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    {
        FeatureCard* pvzbtn = new FeatureCard(pageGame);
        pvzbtn->setFixedSize(312, 200);
        pvzbtn->setName("植物大战僵尸");
        pvzbtn->setDescription("经典老游戏");
        pvzbtn->setIcon(QPixmap(":/Launcher/Imgs/PVZ.png"));
        pvzbtn->setImageUrl(":/Launcher/Imgs/PVZ_bj.jpg");

        FeatureCard* tkabtn = new FeatureCard(pageGame);
        tkabtn->setFixedSize(312, 200);
        tkabtn->setName("致命解药");
        tkabtn->setDescription("这是一款美女打僵尸的游戏");
        tkabtn->setIcon(QPixmap(":/Launcher/Imgs/TKA.png"));
        tkabtn->setImageUrl("https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/2254890/ss_c7cff724b0c4e317d472b12443b19cac48326d36.116x65.jpg");

        // 显示图标
        gameLayout->addWidget(pvzbtn);
        gameLayout->addWidget(tkabtn);
        gameLayout->addStretch();

        // 打开界面
        connect(tkabtn, &FeatureCard::clicked, this, [m_tkaPage, this]() {
            m_tkaPage->setGeometry(rect());
            m_tkaPage->showWithFade();
            });

        connect(pvzbtn, &FeatureCard::clicked, this, [m_pvzPage, this]() {
            m_pvzPage->setGeometry(rect());
            m_pvzPage->showWithFade();
            });
    }

    // STEAM区
    QWidget* pageGame1 = new QWidget();
    QHBoxLayout* game1Layout = new QHBoxLayout(pageGame1);
    game1Layout->setSpacing(10);
    game1Layout->setContentsMargins(10, 10, 10, 10);
    game1Layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    {
        FeatureCard* ysbtn = new FeatureCard(pageGame);
        ysbtn->setFixedSize(312, 200);
        ysbtn->setName("原神");
        ysbtn->setDescription("一款开放世界动作游戏");
        ysbtn->setIcon(QPixmap(":/Launcher/Imgs/Ys.ico"));
        ysbtn->setImageUrl("https://uploadstatic.mihoyo.com/contentweb/20210719/2021071918001232800.jpg");

        FeatureCard* wuwabtn = new FeatureCard(pageGame);
        wuwabtn->setFixedSize(312, 200);
        wuwabtn->setName("鸣潮");
        wuwabtn->setDescription("一款开放世界动作游戏");
        wuwabtn->setIcon(QPixmap(":/Launcher/Imgs/Wuwa.png"));
        wuwabtn->setImageUrl("https://mc.kurogames.com/static4.0/assets/heiHaiAn-09552824.jpg");

        // 显示图标
        game1Layout->addWidget(wuwabtn);
        game1Layout->addWidget(ysbtn);
        game1Layout->addStretch();

        // 打开界面
        connect(wuwabtn, &FeatureCard::clicked, this, [m_wuwaPage, this]() {
            m_wuwaPage->setGeometry(rect());
            m_wuwaPage->showWithFade();
            });

        connect(ysbtn, &FeatureCard::clicked, this, [m_ysPage, this]() {
                m_ysPage->setGeometry(rect());
                m_ysPage->showWithFade();
            });
    }

    tabWidget->addTab("单机专区", pageGame);
    tabWidget->addTab("二游专区", pageGame1);

    m_stack->addWidget(homePage);
    m_stack->addWidget(m_tkaPage);
    m_stack->addWidget(m_wuwaPage);
    m_stack->addWidget(m_ysPage);
    m_stack->setCurrentWidget(homePage);
}