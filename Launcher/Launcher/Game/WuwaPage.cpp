#include "WuwaPage.h"
#include <QVBoxLayout>
#include <QPainter>

WuwaPage::WuwaPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("WuwaPage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    //加载配置
    //const auto& d = miao::GetData();
    //WuwaPach = QString::fromStdString(d.Wuwa_pach);

    // 背景图
    m_bgOriginal.load(DesignSystem::instance()->wuwaImgFilePath());

    // 透明度效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    m_opacityAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_opacityAnim->setDuration(180);
    m_opacityAnim->setEasingCurve(QEasingCurve::InOutCubic);

    hide();

    // 返回按钮
    backBtn = new BackButton(this);
    backBtn->setDiameter(30);

    // 公告
    m_notice = new NoticeCard(this);
    m_notice->setContent("说明:\n1.支持的游戏版本：v3.4.\n2.支持的服务器：国服和国际服.\n3.辅助皆有风险，建议小号.");
    m_notice->resize(310, 190);
    m_notice->raise();

    // 开始按钮
    m_startBtn = new StartButton(this);
    m_startBtn->setLoading(false);
    m_startBtn->setFixedSize(150, 55);
    m_startBtn->raise();

    // 选项按钮
    m_pachBtn = new MoreOptionButton(this);
    m_pachBtn->setFixedSize(52, 52);

    // 登录按钮
    OutlineButton* m_website = new OutlineButton("官方网站", this);
    m_website->setFixedSize(120, 35);
    OutlineButton* m_gamePath = new OutlineButton("游戏路径", this);
    m_gamePath->setFixedSize(120, 35);
    OutlineButton* m_injectType = new OutlineButton("注入类型", this);
    m_injectType->setFixedSize(120, 35);
    OutlineButton* m_modMenu = new OutlineButton("Mod目录", this);
    m_modMenu->setFixedSize(120, 35);
    OutlineButton* m_closeGame = new OutlineButton("强制结束游戏", this);
    m_closeGame->setFixedSize(120, 35);

    m_pachBtn->addMenuButton(m_website);
    m_pachBtn->addMenuButton(m_gamePath);
    m_pachBtn->addMenuButton(m_injectType);
    m_pachBtn->addMenuButton(m_modMenu);
    m_pachBtn->addMenuButton(m_closeGame);

    // 返回主界面
    connect(backBtn, &BackButton::clicked, this, [this]() {
        m_pachBtn->hidePopup(true);
        hideWithFade();
        });

    // 监视控件
    connect(m_website, &QPushButton::clicked, this, [this]()
        {
            QDesktopServices::openUrl(QUrl("https://mc.kurogames.com/main"));
        });

    // MOD界面
    connect(m_modMenu, &QPushButton::clicked, this, [=]() {
        const bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(DesignSystem::instance()->wuwamodPageFilePath()));
        if (!ok) {
            QMessageBox::warning(this, QStringLiteral("无法打开"),
                QStringLiteral("无法打开 MOD 文件夹：\n%1").arg(DesignSystem::instance()->wuwamodPageFilePath()));
        }
        });

    // 开始按钮
    connect(m_startBtn, &QPushButton::clicked, this, [this]()
        {
            if (WuwaPach.isEmpty()) {
                AntMessageManager::instance()->showMessage(AntMessage::Error, "路径不能为空!", 50);
                return;
            }

            m_isLaunching = true;
            m_startBtn->setButtonText("游戏启动中");
            m_startBtn->setLoading(true);

            const std::string pathPach = WuwaPach.toStdString();
            std::wstring dllPach = DesignSystem::instance()->wuwaDllFilePath().toStdWString();
            std::wstring moddllPach = DesignSystem::instance()->wuwamodDllFilePath().toStdWString();

            if (!modBtn)  moddllPach = L"";
            if (!vidaBtn)  dllPach = L"";

            //注入线程
            //std::thread t(Inject::Hijack, pathPach, dllPach, moddllPach);
            //t.detach();
        });

    // 路径界面
    connect(m_gamePath, &QPushButton::clicked, this, [this]()
        {
            PathDialog* dialog = new PathDialog(this);
            dialog->setPath(WuwaPach, "无");
            dialog->showWithAnimation();
            m_pachBtn->hidePopup(true);// 关闭信息框

            connect(dialog, &PathDialog::canceled, this, [=]() {
                dialog->fadeOutAndClose();
                });

            connect(dialog, &PathDialog::confirmed, this, [=]() {
                WuwaPach = dialog->getPath();
                //miao::SaveConfig("Wuwa_pach", WuwaPach.toStdString());
                dialog->fadeOutAndClose();
                });
        });

    // 注入界面
    connect(m_injectType, &QPushButton::clicked, this, [this]()
        {
            InjectDialog* dialog = new InjectDialog(this);

            // 选中状态
            InjectOptions opt;
            opt.vida = vidaBtn;
            opt.mod = modBtn;

            dialog->setOptions(opt);
            dialog->showWithAnimation();
            m_pachBtn->hidePopup(true);// 关闭信息框

            connect(dialog, &InjectDialog::canceled, this, [=]() {
                dialog->fadeOutAndClose();
                });

            connect(dialog, &InjectDialog::confirmed, this, [=]() {
                InjectOptions opt = dialog->getOptions();
                vidaBtn = opt.vida;
                modBtn = opt.mod;

                dialog->fadeOutAndClose();
                });
        });

    // 结束游戏
    connect(m_closeGame, &QPushButton::clicked, this, [this]()
        {
            const bool running = (FindWindowW(nullptr, L"鸣潮  ") != nullptr);
            if (!running) {
                AntMessageManager::instance()->showMessage(AntMessage::Error, "未检测到游戏!", 50);
                return;
            }

            QProcess::execute("taskkill", { "/f", "/im", "Client-Win64-Shipping.exe" });
            AntMessageManager::instance()->showMessage(AntMessage::Success, "已结束游戏!", 50);
        });

    GameProgress(); // 检测游戏进程
}

void WuwaPage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 8;
    QPainterPath path;
    QRect r = rect();

    path.moveTo(r.topLeft());
    path.lineTo(r.topRight() - QPoint(radius, 0));
    path.quadTo(r.topRight(), r.topRight() + QPoint(0, radius));
    path.lineTo(r.bottomRight() - QPoint(0, radius));
    path.quadTo(r.bottomRight(), r.bottomRight() - QPoint(radius, 0));
    path.lineTo(r.bottomLeft());
    path.lineTo(r.topLeft());

    painter.setClipPath(path);

    if (!m_bgOriginal.isNull()) {
        painter.drawPixmap(rect(), m_bgOriginal);
    }
}

void WuwaPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (m_startBtn) {
        const int margin = 210;
        m_startBtn->move(width() - margin, height() - (margin / 2));
    }

    if (m_pachBtn) {
        m_pachBtn->move(width() - m_startBtn->width() - 125,
            height() - m_startBtn->height() - 47);
    }

    if (m_notice) {
        m_notice->move(35, 35);
    }

    if (backBtn) {
        backBtn->move(10, 10);
    }
}

void WuwaPage::GameProgress() {
    //检测游戏进程
    QTimer* m_gameCheckTimer = new QTimer(this);
    m_gameCheckTimer->setInterval(1000);
    connect(m_gameCheckTimer, &QTimer::timeout, this, [this]() {
        //判断是否启动
        const bool running = (FindWindowW(nullptr, L"鸣潮  ") != nullptr);
        if (running) {
            m_isLaunching = false;
            m_startBtn->setButtonText("游戏运行中");
            m_startBtn->setLoading(false, false);

            //软件最小化
            if (m_gameMinimizeCard->isChecked() && !m_hasMinimized) {
                for (QWidget* w : qApp->topLevelWidgets()) {
                    if (auto* main = qobject_cast<MainWindow*>(w)) {
                        main->hide();
                        m_hasMinimized = true;
                        break;
                    }
                }
            }
            return;
        }

        //是否启动失败
        if (m_isLaunching) {
            //if (Ishijacksuccessful == false) {
            //    AntMessageManager::instance()->showMessage(AntMessage::Error, "游戏启动失败,请以管理员身份运行本程序!", 50);
            //    Ishijacksuccessful = true;
            //    m_isLaunching = false;
            //    m_hasMinimized = false;
            //}

            return;
        }

        //还原按钮
        m_startBtn->setButtonText("启动游戏");
        m_startBtn->setLoading(false);
        m_startBtn->setEnabled(true);
        m_hasMinimized = false;
        });
    m_gameCheckTimer->start();
}

void WuwaPage::showWithFade()
{
    if (!m_opacityEffect || !m_opacityAnim)
        return;

    if (m_opacityAnim->state() == QAbstractAnimation::Running)
        m_opacityAnim->stop();

    QObject::disconnect(m_opacityAnim, nullptr, this, nullptr);

    m_isFading = true;
    m_opacityEffect->setOpacity(0.0);

    show();
    raise();
    update();

    m_opacityAnim->setDuration(180);
    m_opacityAnim->setStartValue(0.0);
    m_opacityAnim->setEndValue(1.0);
    m_opacityAnim->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_opacityAnim, &QPropertyAnimation::finished, this, [this]() {
        m_isFading = false;
        m_opacityEffect->setOpacity(1.0);
        });

    m_opacityAnim->start();
}

void WuwaPage::hideWithFade()
{
    if (!m_opacityEffect || !m_opacityAnim) {
        hide();
        return;
    }

    if (!isVisible())
        return;

    if (m_opacityAnim->state() == QAbstractAnimation::Running)
        m_opacityAnim->stop();

    QObject::disconnect(m_opacityAnim, nullptr, this, nullptr);

    m_isFading = true;
    m_opacityEffect->setOpacity(1.0);

    m_opacityAnim->setDuration(160);
    m_opacityAnim->setStartValue(1.0);
    m_opacityAnim->setEndValue(0.0);
    m_opacityAnim->setEasingCurve(QEasingCurve::InCubic);

    connect(m_opacityAnim, &QPropertyAnimation::finished, this, [this]() {
        m_isFading = false;
        hide();
        m_opacityEffect->setOpacity(1.0);
        });

    m_opacityAnim->start();
}