#pragma once
#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <qtimer>
#include <qmenu.h>
#include <qpointer.h>
#include <QPainter>
#include <QScrollBar>
#include <qlineedit>
#include <QVBoxLayout>
#include <QSplitter>
#include <QClipboard>
#include <QGuiApplication>
#include <qscrollarea.h>
#include <qtextbrowser.h>
#include <QRegularExpression>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStackedWidget>
#include <QScroller>

#include "EmojiPanel.h"
#include "../Control/AntChatListView.h"
#include "../Control/SendBarWidget.h"
#include "../Control/CustomToolButton.h"
#include "../Control/CustomButton.h"
#include "../Control/SmoothScrollBar.h"
#include "../Design/AntMessageManager.h"
#include "../Globals/Globals.h"
#include "../Design/DesignSystem.h"

extern QString m_selfName;
extern bool b_initSocket;
extern bool m_isLogin;

struct ChatUi {
    QScrollArea* scroll = nullptr;
    QWidget* container = nullptr;
    QVBoxLayout* msgLay = nullptr;

    bool stickToBottom = true;
    bool programmaticScroll = false;
};

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget* parent = nullptr);

    QWidget* createChatPage(const QString& title, int chatIndex);
    QLabel* appendMessage(int chatIndex, const QString& userName, const QString& content, const QString& avatarPath);
    void sendCurrentMessage();
    void scrollToBottom(int chatIndex);
    void clearChatUI(int chatIndex);
    void updateinformation();
    void resizeEvent(QResizeEvent* event);
    void AIchat(const QString& text, QPointer<QLabel> bubble);

protected:

private:
    bool eventFilter(QObject* obj, QEvent* event);
    void showEmojiPanel(CustomToolButton* btnExpression);

private:
    AntChatListView* m_chatList = nullptr;
    QStandardItemModel* m_listModel = nullptr;
    QStackedWidget* m_stack = nullptr;
    QLineEdit* m_input = nullptr;
    EmojiPanel* m_emojiPanel = nullptr;
    QSplitter* splitter = nullptr;
    SendBarWidget* sendBar = nullptr;
    QWidget* rightPanel = nullptr;

    // 当前聊天页的消息布局
    QVBoxLayout* m_messageLayout = nullptr;
    QMap<int, QVBoxLayout*> m_messageLayouts;
    QMap<int, ChatUi> m_chatUis;
    QVector<AntChatListView::ChatItem> chatItems;

    QString m_selfAvatar = QStringLiteral(":/Launcher/Imgs/UserAvatar.png");
    QString m_AiAvatar = QStringLiteral(":/Launcher/Imgs/deepseek.png");

    int userCount = 0;
    size_t m_lastMsgIndex = 0;
    std::string Aiinput;
    QTimer* m_gameCheckTimer = nullptr;

    //ai
    QNetworkAccessManager* manager;

    CustomToolButton* btnExpression = nullptr;
};