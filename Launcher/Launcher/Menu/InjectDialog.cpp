#include "InjectDialog.h"

InjectDialog::InjectDialog(QWidget* parent)
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
    QLabel* pathLabel = new QLabel("注入类型:", m_panel);
    pathLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    // 输入框
    m_dllCheck = new CapsuleCheckBox("Vida", m_panel);
    m_dllCheck->setFixedSize(200, 23);
    m_modCheck = new CapsuleCheckBox("Mod", m_panel);
    m_modCheck->setFixedSize(200, 23);

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
    contentLayout->addWidget(m_dllCheck);
    contentLayout->addWidget(m_modCheck);

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
    connect(btnOk, &QPushButton::clicked, this, &InjectDialog::confirmed);
    connect(btnCancel, &QPushButton::clicked, this, &InjectDialog::canceled);

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

void InjectDialog::resizeEvent(QResizeEvent*)
{
    if (parentWidget())
        setGeometry(parentWidget()->rect());

    if (m_blockMask)
        m_blockMask->setGeometry(rect());

    updatePanelLayout();
}

void InjectDialog::paintEvent(QPaintEvent* event)
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

void InjectDialog::updatePanelLayout()
{
    int w = m_panel->width();
    int h = m_panel->height();

    QPoint center((width() - w) / 2, (height() - h) / 2);
    m_panel->move(center.x(), center.y() - m_panelOffsetY);
}

void InjectDialog::showWithAnimation()
{
    if (m_animating) return;

    m_closing = false;
    m_opening = true;
    m_animating = true;

    m_progress = 0.0;
    m_maskOpacity = 0.0;
    m_panelOpacity = 0.0;
    m_panelOffsetY = 35;
    m_useSnapshot = false;

    if (parentWidget())
        setGeometry(parentWidget()->rect());

    updatePanelLayout();

    show();
    raise();

    m_panel->show();
    m_panel->raise();
    updatePanelLayout();

    m_elapsedTimer.restart();
    m_frameTimer->start();
}

void InjectDialog::fadeOutAndClose()
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

void InjectDialog::setOptions(const InjectOptions& options)
{
    if (m_dllCheck)
        m_dllCheck->setChecked(options.vida);

    if (m_modCheck)
        m_modCheck->setChecked(options.mod);
}

InjectOptions InjectDialog::getOptions() const
{
    InjectOptions opt;

    if (m_dllCheck)
        opt.vida = m_dllCheck->isChecked();

    if (m_modCheck)
        opt.mod = m_modCheck->isChecked();

    return opt;
}