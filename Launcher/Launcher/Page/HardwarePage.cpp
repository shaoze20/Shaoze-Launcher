#include "HardwarePage.h"

HardwarePage::HardwarePage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("HardwarePage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(0);

    CustomTabWidget* tabWidget = new CustomTabWidget(this);

    // 硬件检测
    QWidget* pageHardware = new QWidget();
    QVBoxLayout* hardwareLayout = new QVBoxLayout(pageHardware);
    hardwareLayout->setContentsMargins(0, 0, 0, 0);
    hardwareLayout->setSpacing(0);
    {
        QWidget* contentWidget = new QWidget();
        contentWidget->setObjectName("contentWidget");
        contentWidget->setStyleSheet(R"(
            QWidget#contentWidget {
                background: transparent;
            }
        )");

        QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(0, 0, 5, 0);
        contentLayout->setSpacing(10);

        // 横向布局
        QHBoxLayout* topRowLayout = new QHBoxLayout();
        topRowLayout->setSpacing(10);
        topRowLayout->setContentsMargins(0, 0, 0, 0);

        InfoCard* systemCard = new InfoCard("系统信息", HardwareInfo::systemInfo(), contentWidget);
        m_uptimeCard = new InfoCard("运行时间", HardwareInfo::uptime(), contentWidget);

        topRowLayout->addWidget(systemCard, 1);
        topRowLayout->addWidget(m_uptimeCard, 1);

        // 加入到主内容布局
        contentLayout->addLayout(topRowLayout);

        contentLayout->addWidget(new InfoCard("处理器", HardwareInfo::cpuName(), contentWidget));
        contentLayout->addWidget(new InfoCard("主板", HardwareInfo::motherboard(), contentWidget));

        QString memoryText = QString("总内存：%1").arg(HardwareInfo::totalMemory());
        const auto mems = HardwareInfo::memoryModules();
        for (const auto& mem : mems) {
            memoryText += "\n";
            memoryText += QString("• %1  %2  %3  %4")
                .arg(mem.manufacturer.isEmpty() ? "Unknown" : mem.manufacturer)
                .arg(mem.partNumber.isEmpty() ? "-" : mem.partNumber)
                .arg(mem.capacity.isEmpty() ? "-" : mem.capacity)
                .arg(mem.speed.isEmpty() ? "-" : mem.speed);
        }
        contentLayout->addWidget(new InfoCard("内存", memoryText, contentWidget));

        QString gpuText;
        const QStringList gpus = HardwareInfo::graphicsCards();
        gpuText = gpus.isEmpty() ? "未检测到显卡信息" : gpus.join("\n");
        contentLayout->addWidget(new InfoCard("显卡", gpuText, contentWidget));

        QString monitorText;
        const QStringList monitors = HardwareInfo::monitors();
        monitorText = monitors.isEmpty() ? "未检测到显示器信息" : monitors.join("\n");
        contentLayout->addWidget(new InfoCard("显示器", monitorText, contentWidget));

        QString diskText;
        const QStringList disks = HardwareInfo::diskDrives();
        diskText = disks.isEmpty() ? "未检测到磁盘信息" : disks.join("\n");
        contentLayout->addWidget(new InfoCard("磁盘", diskText, contentWidget));

        QString soundText;
        const QStringList sounds = HardwareInfo::soundCards();
        soundText = sounds.isEmpty() ? "未检测到声卡信息" : sounds.join("\n");
        contentLayout->addWidget(new InfoCard("声卡", soundText, contentWidget));

        QString networkText;
        const QStringList nets = HardwareInfo::networkCards();
        networkText = nets.isEmpty() ? "未检测到网卡信息" : nets.join("\n");
        contentLayout->addWidget(new InfoCard("网卡", networkText, contentWidget));

        contentLayout->addStretch();

        QScrollArea* scrollArea = new QScrollArea(pageHardware);
        SmoothScrollBar* vBar = new SmoothScrollBar(Qt::Vertical, scrollArea);
        scrollArea->setVerticalScrollBar(vBar);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidget(contentWidget);

        hardwareLayout->addWidget(scrollArea);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, [this]() {
            if (m_uptimeCard) {
                m_uptimeCard->setContent(HardwareInfo::uptime());
            }
            });
        m_timer->start(1000);
    }

    // 检测工具
    QWidget* pageTools = new QWidget();
    QVBoxLayout* toolsLayout = new QVBoxLayout(pageTools);
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    {

    }

    // 添加选项卡
    tabWidget->addTab("硬件检测", pageHardware);
    tabWidget->addTab("系统工具", pageTools);

    m_mainLayout->addWidget(tabWidget);
}