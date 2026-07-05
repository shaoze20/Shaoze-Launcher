#pragma once

#include <QPixmapCache>
#include "AntBaseListView.h"
#include "../Globals/Globals.h"

class AntChatListItemDelegate : public AntBaseListItemDelegate
{
public:
	explicit AntChatListItemDelegate(QObject* parent = nullptr)
		: AntBaseListItemDelegate(parent)
	{
	}

	static QPixmap cachedRoundedAvatar(const QString& imagePath, int avatarSize, int radius, qreal dpr)
	{
		const QString key = imagePath + "|"
			+ QString::number(avatarSize) + "|"
			+ QString::number(radius) + "|"
			+ QString::number(dpr, 'f', 2);

		QPixmap cached;
		if (QPixmapCache::find(key, &cached)) {
			return cached;
		}

		QPixmap src(imagePath);
		if (src.isNull()) {
			return QPixmap();
		}

		// 用你现有的高 DPI 函数
		QPixmap scaled = Globals::makeHighDpiPixmap(
			src,
			QSize(avatarSize, avatarSize),
			dpr
		);

		// 输出画布也要按高 DPI 创建
		QPixmap out(qRound(avatarSize * dpr), qRound(avatarSize * dpr));
		out.fill(Qt::transparent);
		out.setDevicePixelRatio(dpr);

		QPainter p(&out);
		p.setRenderHint(QPainter::Antialiasing, true);
		p.setRenderHint(QPainter::SmoothPixmapTransform, true);

		QPainterPath clip;
		clip.addRoundedRect(QRectF(0, 0, avatarSize, avatarSize), radius, radius);
		p.setClipPath(clip);

		// 用逻辑尺寸做居中
		QSize drawSize = scaled.deviceIndependentSize().toSize();
		const qreal x = (avatarSize - drawSize.width()) / 2.0;
		const qreal y = (avatarSize - drawSize.height()) / 2.0;

		p.drawPixmap(QPointF(x, y), scaled);

		QPixmapCache::insert(key, out);
		return out;
	}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		const QString name = index.data(Qt::UserRole + 1).toString();
		const QString message = index.data(Qt::UserRole + 2).toString();

		// 设置用户名字体（加粗）
		QFont nameFont = option.font;
		nameFont.setBold(true);
		QFontMetrics nameMetrics(nameFont);
		int nameHeight = nameMetrics.height();

		// 普通字体用于消息
		QFont messageFont = option.font;
		QFontMetrics messageMetrics(messageFont);
		int messageHeight = messageMetrics.height();

		// 垂直方向
		const int verticalPadding = 8;
		const int spacingBetweenLines = 4;

		// 总高度
		int textBlockHeight = verticalPadding + nameHeight + spacingBetweenLines + messageHeight + verticalPadding;

		// 头像高度
		const int avatarHeight = 50;

		// 返回高度取文本区与头像中较高者
		int totalHeight = qMax(textBlockHeight, avatarHeight);

		return QSize(option.rect.width(), totalHeight);
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		AntBaseListItemDelegate::paint(painter, option, index);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		QRect rect = option.rect;

		const int imageSize = 46;
		int imgMargin = (rect.height() - imageSize) / 2;
		QRect imageRect(rect.left() + imgMargin, rect.top() + imgMargin, imageSize, imageSize);

		// 头像尺寸
		const int avatarSize = 42;
		QRect avatarRect(
			imageRect.left() + (imageRect.width() - avatarSize) / 2,
			imageRect.top() + (imageRect.height() - avatarSize) / 2,
			avatarSize, avatarSize
		);

		// 绘制头像
		QString imagePath = index.data(Qt::DecorationRole).toString();
		qreal dpr = painter->device()->devicePixelRatioF();
		QPixmap avatarPixmap = AntChatListItemDelegate::cachedRoundedAvatar(imagePath, avatarSize, avatarSize / 2, dpr);
		painter->drawPixmap(avatarRect.topLeft(), avatarPixmap);

		// 根据头像位置绘制文本区域
		int textLeft = imageRect.right() + imgMargin;
		int textWidth = rect.right() - textLeft - 10;
		QRect textRect(textLeft, rect.top(), textWidth, rect.height());

		QString userName = index.data(Qt::UserRole + 1).toString();
		QString message = index.data(Qt::UserRole + 2).toString();

		// 字体和字体度量
		QFont userNameFont = option.font;
		userNameFont.setBold(true);
		userNameFont.setPointSizeF(userNameFont.pointSizeF() + 0.5);  // 放大0.5pt
		QFontMetrics fmUserName(userNameFont);
		int userNameHeight = fmUserName.height();

		QFont messageFont = option.font;
		messageFont.setPointSizeF(messageFont.pointSizeF() - 0.5);     // 缩小0.5pt
		QFontMetrics fmMessage(messageFont);
		int messageHeight = fmMessage.height();

		QColor original = QColor(0, 0, 0);
		QColor lighter = QColor(150, 150, 150);

		// 用户名顶部对齐头像顶部
		QRect userNameRect(textRect.left(), imageRect.top() , textWidth, userNameHeight);
		QString elidedUserName = fmUserName.elidedText(userName, Qt::ElideRight, textWidth);
		painter->setFont(userNameFont);
		painter->setPen(original);
		painter->drawText(userNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedUserName);

		// 消息底部对齐头像底部，单行省略
		QRect messageRect(textRect.left(), imageRect.bottom() - messageHeight, textWidth, messageHeight);
		QString elidedMessage = fmMessage.elidedText(message, Qt::ElideRight, textWidth);
		painter->setFont(messageFont);
		painter->setPen(lighter);
		painter->drawText(messageRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMessage);

		painter->restore();
	}

	void updateStyle()
	{
		updateBaseStyle();
	}
};

class AntChatListView : public AntBaseListView
{
	Q_OBJECT

public:
	// 数据
	struct ChatItem
	{
		QString avatarPath;  // 头像路径
		QString name;        // 昵称
		QString message;     // 消息内容
		QString time;        // 消息时间
		bool isSentByMe;     // 是否是自己发送的
	};

	AntChatListView(QWidget* parent);
	~AntChatListView();
	// 创建模型
	QStandardItemModel* createModel(const QVector<ChatItem>& chatItems);
private:
	AntChatListItemDelegate* itemDele;
};
