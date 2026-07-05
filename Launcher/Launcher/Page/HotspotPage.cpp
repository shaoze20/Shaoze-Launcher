#include "HotspotPage.h"
#include <QVBoxLayout>
#include <QLabel>

HotspotPage::HotspotPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("HotspotPage");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 10);
    mainLayout->setSpacing(0);

    // 滚动条
    QScrollArea* scrollArea = new QScrollArea(this);
    SmoothScrollBar* vBar = new SmoothScrollBar(Qt::Vertical, scrollArea);
    scrollArea->setVerticalScrollBar(vBar);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: none;
            background: transparent;
        }
    )");

    // 内容的容器
    QWidget* contentWidget = new QWidget();
    contentWidget->setObjectName("contentWidget");
    contentWidget->setStyleSheet(R"(
        QWidget#contentWidget {
            background: transparent;
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(0, 0, 0, 5);
    layout->setSpacing(0);

    // 轮播图
    BannerCardWidget* card = new BannerCardWidget(this);
    card->setFixedSize(980, 405);

    QVector<BannerCardWidget::BannerItem> items = {
        {
            "GitHub",
            "全球最大代码托管平台",
            "https://github.com",
            "https://github.githubassets.com/images/modules/memexes/projects-beta-banner.png"

        },
        {
            "火绒安全",
            "非常轻量化的一款杀毒软件,后台占用及少",
            "https://www.huorong.cn/",
            "https://cdn-www.huorong.cn/Public/Uploads/uploadfile/images/20250729/qiyebanwebduanbeijing.png",
        },
        {
            "图吧工具箱",
            "专业且简洁的硬件检测工具",
            "https://www.tbtool.cn/",
            "https://www.tbtool.cn/rc_images/aboutimg.png",
        },
        {
            "Voicemeeter",
            "强大的音频管理软件",
            "https://vb-audio.com/Voicemeeter/",
            "https://vb-audio.com/Voicemeeter/VoicemeeterAudioMixer.jpg",
        }
    };
    card->setItems(items);

    // 卡片
    HotSiteCard* card1 = new HotSiteCard(
        "Cheat Engine",
        "强大的游戏修改工具,可以修改任何软件和游戏",
        "热度 9.1",
        "https://www.cheatengine.org/",
        "",
        this
    );

    HotSiteCard* card2 = new HotSiteCard(
        "Neatdownload",
        "非常好用的下载工具,和IDM的功能基本差不多且完全免费",
        "热度 9.1",
        "https://neatdownload.com/",
        "",
        this
    );

    HotSiteCard* card3 = new HotSiteCard(
        "音乐格式转换工具",
        "可以免费转换网易云、酷狗、qq音乐等软件",
        "热度 9.1",
        "https://a.91mp3.top/",
        "https://a.91mp3.top/img/icons/favicon-32x32.png",
        this
    );

    HotSiteCard* card4 = new HotSiteCard(
        "精易论坛",
        "易语言开发者论坛,有模块、成品、软件等",
        "热度 9.1",
        "https://bbs.ijingyi.com/",
        "",
        this
    );

    // 添加主界面
    layout->addWidget(card);
    layout->addWidget(card1);
    layout->addWidget(card2);
    layout->addWidget(card3);
    layout->addWidget(card4);
    layout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}