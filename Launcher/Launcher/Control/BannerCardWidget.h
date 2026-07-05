#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QTimer>
#include <QPropertyAnimation>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHash>

#include "../Control/CustomButton.h"

class BannerCardWidget : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)
        Q_PROPERTY(qreal dotOffset READ dotOffset WRITE setDotOffset)

public:
    struct BannerItem
    {
        QString name;
        QString description;
        QString officialUrl;
        QString imageUrl;
    };

    explicit BannerCardWidget(QWidget* parent = nullptr);

    void setItems(const QVector<BannerItem>& items);
    void setCurrentIndex(int index);
    int currentIndex() const { return m_currentIndex; }

    qreal slideOffset() const { return m_slideOffset; }
    void setSlideOffset(qreal offset);

    qreal dotOffset() const { return m_dotOffset; }
    void setDotOffset(qreal offset);

signals:
    void officialClicked(const QString& url);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void nextSlide();
    void onImageDownloaded(QNetworkReply* reply);

private:
    void startSlideAnimation(int nextIndex);
    void drawSlideContent(QPainter& painter, const QRectF& rect, int itemIndex);
    void drawOverlay(QPainter& painter, const QRectF& cardRect);
    void ensureImageLoaded(int index);
    QRect officialButtonRect(const QRectF& cardRect) const;
    QVector<QRect> dotRects(const QRectF& cardRect) const;

private:
    QWidget* m_btnContainer = nullptr;
    CustomButton* m_btnCurrent = nullptr;
    CustomButton* m_btnNext = nullptr;
    QVector<BannerItem> m_items;
    QHash<QString, QPixmap> m_pixmapCache;
    QHash<QNetworkReply*, QString> m_replyUrlMap;

    int m_currentIndex = 0;
    int m_nextIndex = -1;
    qreal m_slideOffset = 0.0;

    QTimer* m_timer = nullptr;
    QPropertyAnimation* m_slideAnim = nullptr;
    QNetworkAccessManager* m_net = nullptr;

    qreal m_dotOffset = 0.0;
    QPropertyAnimation* m_dotAnim = nullptr;
    int m_dotFromIndex = 0;
    int m_dotToIndex = 0;
};