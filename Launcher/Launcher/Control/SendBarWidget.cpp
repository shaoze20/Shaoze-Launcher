#include "SendBarWidget.h"
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

SendBarWidget::SendBarWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);

    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(10, 0, 12, 10);
    grid->setHorizontalSpacing(0);
    grid->setVerticalSpacing(0);

    // 左上区域
    QWidget* topLeftBox = new QWidget(this);
    topLeftBox->setAttribute(Qt::WA_TranslucentBackground);
    m_topLeftLay = new QHBoxLayout(topLeftBox);
    topLeftBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    topLeftBox->setContentsMargins(0, 0, 0, 0);
    m_topLeftLay->setSpacing(0);
    m_topLeftLay->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 输入框 
    m_input = new QPlainTextEdit(this);
    m_input->setPlaceholderText("请输入消息...(Enter 发送,Shift + Enter 换行)");
    m_input->setFrameShape(QFrame::NoFrame);
    m_input->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_input->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setStyleSheet("background: transparent; color: #000; font-size: 15px;");
    m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_input->setFixedHeight(m_minEditH);
    m_input->installEventFilter(this);

    // 右下区域
    QWidget* bottomRightBox = new QWidget(this);
    bottomRightBox->setAttribute(Qt::WA_TranslucentBackground);
    m_bottomRightLay = new QHBoxLayout(bottomRightBox);
    m_bottomRightLay->setContentsMargins(10, 10, 10, 10);
    m_bottomRightLay->setSpacing(6);

    grid->addWidget(bottomRightBox, 2, 1, 1, 1, Qt::AlignRight | Qt::AlignBottom);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);

    connect(m_input, &QPlainTextEdit::textChanged, this, [this] {
        adjustInputHeight();
        });

    QWidget* leftBox = new QWidget(this);
    leftBox->setAttribute(Qt::WA_TranslucentBackground);
    leftBox->setStyleSheet("background: transparent;");

    auto* leftLay = new QVBoxLayout(leftBox);
    leftLay->setContentsMargins(0, 5, 0, 0);
    leftLay->setSpacing(0);

    m_topLeftLay->setContentsMargins(0, 0, 0, 0);
    m_topLeftLay->setSpacing(6);

    leftLay->addWidget(topLeftBox, 0);
    leftLay->addSpacing(5);
    leftLay->addWidget(m_input, 0);
    leftLay->addStretch(1);

    grid->addWidget(leftBox, 0, 0, 3, 1, Qt::AlignTop);

    // 阴影
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);                 // 模糊程度
    shadow->setOffset(0, 4);                   // 向下偏移（更自然）
    shadow->setColor(QColor(0, 0, 0, 80));     // 半透明黑

    this->setGraphicsEffect(shadow);
}

bool SendBarWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_input && event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) &&
            !(ke->modifiers() & Qt::ShiftModifier))
        {
            const QString text = m_input->toPlainText().trimmed();
            if (!text.isEmpty()) {
                emit sendRequested(text);
                m_input->clear();
                adjustInputHeight();
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SendBarWidget::adjustInputHeight()
{
    const int docH = int(m_input->document()->size().height());
    const int target = qBound(m_minEditH, docH + 6, m_maxEditH);
    m_input->setFixedHeight(target);
}

void SendBarWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF r = rect();
    r.adjust(0.5, 0.5, -0.5, -0.5);

    QPainterPath path;
    path.addRoundedRect(r, m_radius, m_radius);

    // 纯白背景
    QColor bg(255, 255, 255, 255);
    p.fillPath(path, bg);

    //边框
    QPen pen(QColor(220, 220, 220)); // 边框颜色
    pen.setWidth(1);                 // 细边框
    p.setPen(pen);
    p.drawPath(path);
}