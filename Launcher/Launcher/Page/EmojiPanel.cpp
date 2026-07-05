#include "EmojiPanel.h"

EmojiPanel::EmojiPanel(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet(StyleSheet::vListViewQss(QColor(220, 220, 220)));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 10 + m_arrowH);
    root->setSpacing(0);

    // ScrollArea
    m_scroll = new QScrollArea(this);
    SmoothScrollBar* vBar = new SmoothScrollBar(Qt::Vertical, m_scroll);
    m_scroll->setVerticalScrollBar(vBar);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll->setStyleSheet("background: transparent;");

    m_scroll->setFixedHeight(m_fixedContentH);

    QWidget* content = new QWidget;
    content->setAttribute(Qt::WA_TranslucentBackground);
    content->setStyleSheet("background: transparent;");

    auto* lay = new QGridLayout(content);
    lay->setContentsMargins(0, 0, 10, 0);
    lay->setSpacing(6);
    m_scroll->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    lay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    const QStringList emojis = {
        "😀","😁","😂","🤣","😃","😄","😅","😆",
        "😉","😊","🙂","🙃","😍","😘","😗","😙",
        "😚","🤗","🤔","😐","😑","😶","🙄","😏",
        "😣","😥","😮","🤐","😯","😪","😫","😴",
        "😌","😛","😜","😝","🤤","😒","😓","😔",
        "😕","🙁","☹️","😖","😞","😟","😢","😭",
        "😤","😠","😡","🤬","🤯","😳","🥵","🥶",
        "😱","😨","😰","😲","😵","🤪","😇","🐸"
        "👍","👎","👌","✌️","🤞","🤟","🤘","🤙",
        "👏","🙌","👐","🤲","🙏","💪","🦾","🐵",
        "❤️","🧡","💛","💚","💙","💜","🤎","🖤",
        "🤍","💔","❣️","💕","💞","💓","💗","💖",
        "💘","💝","🔥","💯","✨","⭐","🌟","⚡",
        "💥","💫","🎉","🎊","🎈","🎁","📌","📍",
        "✅","❌","❗","❓","🤡","😎","🤓","🧐",
        "😬","🙃","🤨", "🐶","🐱","🐭","🐹","🐰",
        "🦊","🐻","🐼","🐨","🐯","🦁","🐮","🐷"
    };

    int r = 0, c = 0;
    for (const auto& e : emojis) {
        auto* btn = new QToolButton(content);
        btn->setText(e);
        btn->setFixedSize(50, 50);
        btn->setStyleSheet(R"(
            QToolButton {
                background: transparent;
                border: none;
                font-size: 25px;
            }
            QToolButton:hover {
                background: rgba(0,0,0,0.15);
                border-radius: 15px;
            }
        )");

        connect(btn, &QToolButton::clicked, this, [this, e]() {
            emit emojiClicked(e);
            hide();
            });

        lay->addWidget(btn, r, c);
        if (++c >= 8) { c = 0; ++r; }
    }

    m_scroll->setWidget(content);
    root->addWidget(m_scroll);

    adjustSize();
    resize(sizeHint().width(), m_fixedContentH + 10 + (10 + m_arrowH));
}

void EmojiPanel::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor bg(240, 240, 240, 240);

    // 主体区域（不含三角）
    QRect body = rect();
    body.adjust(0, 0, 0, -m_arrowH);

    // 圆角矩形
    QPainterPath path;
    path.addRoundedRect(body, m_radius, m_radius);

    // 底部居中三角
    const int cx = (m_arrowCenterX >= 0) ? m_arrowCenterX : body.center().x();
    QPolygon arrow;
    arrow << QPoint(cx - m_arrowW / 2, body.bottom())
        << QPoint(cx + m_arrowW / 2, body.bottom())
        << QPoint(cx, body.bottom() + m_arrowH);
    path.addPolygon(arrow);

    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawPath(path);
}

void EmojiPanel::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    emit panelClosed();
}