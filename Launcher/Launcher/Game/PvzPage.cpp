#include "PvzPage.h"
#include <QVBoxLayout>
#include <QPainter>

PvzPage::PvzPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("PvzPage");
    setStyleSheet(StyleSheet::background(DesignSystem::instance()->color.background));

    // 透明度效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    m_opacityAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_opacityAnim->setDuration(180);
    m_opacityAnim->setEasingCurve(QEasingCurve::InOutCubic);

    hide();

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 50, 10, 10);
    mainLayout->setSpacing(10);

    // 上区域
    m_topPanel = new QWidget(this);
    m_topPanel->setObjectName("PvzProcessPanel");
    m_topPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_topPanel->setMinimumHeight(50);

    // 上区域布局
    QHBoxLayout* topLayout = new QHBoxLayout(m_topPanel);
    topLayout->setContentsMargins(15, 10, 10, 10);

    // 状态文本
    m_statusLabel = new QLabel("游戏状态：未检测", m_topPanel);
    m_statusLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    // 样式
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: black;
            font-weight: bold;
            font-size: 15px;
        }
    )");

    // 进度条
    m_spinner = new MaterialSpinner(QSize(30, 30), 4, QColor(124, 252, 0), this);
    m_spinner->hide();

    topLayout->addWidget(m_statusLabel);
    topLayout->addStretch(); // 保证靠左
    topLayout->addWidget(m_spinner);

    // 下区域
    m_bottomPanel = new QWidget(this);
    m_bottomPanel->setObjectName("PvzFunctionPanel");
    m_bottomPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_bottomPanel->setMinimumHeight(220);

    // 下区域布局
    QVBoxLayout* bottomLayout = new QVBoxLayout(m_bottomPanel);
    bottomLayout->setContentsMargins(10, 10, 10, 10);
    bottomLayout->setSpacing(10);

    // 功能
    QHBoxLayout* row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(10);
    m_ModifysunlightBtn = new CustomButton("修改阳光", m_bottomPanel);
    m_ModifysunlightBtn->setFixedSize(90, 32);
    m_AmountsunlightEdit = new CustomLineEdit(m_bottomPanel);
    m_AmountsunlightEdit->setPlaceholderText("请输入阳光数量");
    m_AmountsunlightEdit->setValidator(new QIntValidator(0, 999999, m_AmountsunlightEdit));
    m_AmountsunlightEdit->setFixedSize(180, 32);
    row->addWidget(m_ModifysunlightBtn);
    row->addWidget(m_AmountsunlightEdit);
    row->addStretch();

    QHBoxLayout* row1 = new QHBoxLayout;
    row1->setContentsMargins(0, 0, 0, 0);
    row1->setSpacing(10);
    m_ModifyCoinsBtn = new CustomButton("修改金币", m_bottomPanel);
    m_ModifyCoinsBtn->setFixedSize(90, 32);
    m_NumbercoinsEdit = new CustomLineEdit(m_bottomPanel);
    m_NumbercoinsEdit->setPlaceholderText("请输入金币数量");
    m_NumbercoinsEdit->setValidator(new QIntValidator(0, 999999, m_NumbercoinsEdit));
    m_NumbercoinsEdit->setFixedSize(180, 32);
    row1->addWidget(m_ModifyCoinsBtn);
    row1->addWidget(m_NumbercoinsEdit);
    row1->addStretch();

    int btnW = 200, btnH = 23;
    m_SunlightRestrictionCheck = new CapsuleCheckBox("去除阳光限制", m_bottomPanel);
    m_SunlightRestrictionCheck->setFixedSize(btnW, btnH);
    m_NoCdCheck = new CapsuleCheckBox("卡槽无冷却", m_bottomPanel);
    m_NoCdCheck->setFixedSize(btnW, btnH);

    bottomLayout->addLayout(row);
    bottomLayout->addLayout(row1);
    bottomLayout->addWidget(m_SunlightRestrictionCheck);
    bottomLayout->addWidget(m_NoCdCheck);
    bottomLayout->addStretch();

    // 区域样式
    m_topPanel->setStyleSheet(R"(
        QWidget#PvzProcessPanel {
            background: white;
            border: 1px solid #D9D9D9;
            border-radius: 8px;
        }
    )");

    m_bottomPanel->setStyleSheet(R"(
        QWidget#PvzFunctionPanel {
            background: white;
            border: 1px solid #D9D9D9;
            border-radius: 8px;
        }
    )");

    mainLayout->addWidget(m_topPanel);
    mainLayout->addWidget(m_bottomPanel, 1);

    // 返回按钮
    backBtn = new BackButton(this);
    backBtn->setDiameter(30);

    // 返回主界面
    connect(backBtn, &BackButton::clicked, this, [this]() {
        hideWithFade();
        });

    GameProgress(); // 检测游戏进程
}

void PvzPage::paintEvent(QPaintEvent* event)
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
    painter.fillPath(path, DesignSystem::instance()->color.background);
}

void PvzPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (backBtn) {
        backBtn->move(10, 10);
    }
}

void PvzPage::setGameStatus(const QString& status)
{
    if (m_statusLabel) {
        m_statusLabel->setText("游戏状态：" + status);
    }
}

//检测游戏状态
void PvzPage::GameProgress() {
    QTimer* m_gameCheckTimer = new QTimer(this);
    m_gameCheckTimer->setInterval(200);
    connect(m_gameCheckTimer, &QTimer::timeout, this, [this]() {
        static HANDLE hProcess = NULL;

        HWND hWnd = FindWindowA("MainWindow", NULL);
        if (hWnd) {
            setGameStatus("进程获取完成!");
            m_spinner->hide();

            //// 取进程ID
            //if (!hProcess) {
            //    hProcess = miao::GetProcessHandleByName(L"PlantsVsZombies.exe");
            //    if (!hProcess){
            //        setGameStatus("进程打开失败!");
            //        return;
            //    }
            //}

            //// 修改阳光
            //if(m_ModifysunlightBtn->isDown()){
            //    auto Sunlight = miao::ReadPointerAddress(hProcess, 0x6A9EC0, { 0x768, 0x5560 });
            //    miao::WriteMemoryValueSafe<int>(hProcess, Sunlight, m_AmountsunlightEdit->text().toInt());
            //}

            //// 修改金币
            //if (m_ModifyCoinsBtn->isDown()) {
            //    auto GoldCoin = miao::ReadPointerAddress(hProcess, 0x6A9EC0, { 0x82C, 0x28 });
            //    miao::WriteMemoryValueSafe<int>(hProcess, GoldCoin, m_NumbercoinsEdit->text().toInt());
            //}

            //// 去除阳光限制
            //if (m_SunlightRestrictionCheck->isChecked()) {
            //    miao::WriteMemoryBytesSafe(hProcess, 0x430A23, std::vector<BYTE>{ 0xEB, 0x78 });
            //    miao::WriteMemoryBytesSafe(hProcess, 0x430A78, std::vector<BYTE>{ 0xEB, 0x09});
            //    miao::WriteMemoryBytesSafe(hProcess, 0x48CAB0, std::vector<BYTE>{ 0xEB, 0x09 });
            //}
            //else{
            //    miao::WriteMemoryBytesSafe(hProcess, 0x430A23, std::vector<BYTE>{ 0x7E, 0x78 });
            //    miao::WriteMemoryBytesSafe(hProcess, 0x430A78, std::vector<BYTE>{ 0x7E, 0x09});
            //    miao::WriteMemoryBytesSafe(hProcess, 0x48CAB0, std::vector<BYTE>{ 0x7E, 0x09 });
            //}

            //// 卡槽无CD
            //if (m_NoCdCheck->isChecked()) {
            //    miao::WriteMemoryBytesSafe(hProcess, 0x00488250, std::vector<BYTE>{ 0xEB, 0x24 });
            //    miao::WriteMemoryBytesSafe(hProcess, 0x00488688, std::vector<BYTE>{ 0xE9, 0x7A, 0x01, 0x00, 0x00, 0x90});
            //}
            //else{
            //    miao::WriteMemoryBytesSafe(hProcess, 0x00488250, std::vector<BYTE>{ 0x75, 0x24 });
            //    miao::WriteMemoryBytesSafe(hProcess, 0x00488688, std::vector<BYTE>{ 0x0F, 0x85, 0x79, 0x01, 0x00, 0x00});
            //}

        }
        else{
            if (hProcess) {
                CloseHandle(hProcess);
                hProcess = NULL;
            }

            setGameStatus("未检测到游戏进程!");
            m_spinner->show();
        }
        });

    m_gameCheckTimer->start();
}

void PvzPage::showWithFade()
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

void PvzPage::hideWithFade()
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