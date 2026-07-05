#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QEnterEvent>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class HotSiteCard : public QWidget
{
    Q_OBJECT

public:
    explicit HotSiteCard(const QString& name,
        const QString& description,
        const QString& heatText,
        const QString& url,
        const QString& iconSource,
        QWidget* parent = nullptr);

    void setSiteName(const QString& name);
    void setDescription(const QString& description);
    void setHeatText(const QString& heatText);
    void setIcon(const QPixmap& icon);
    void setIcon(const QString& iconSource);

    QString siteName() const;
    QString description() const;
    QString heatText() const;
    QPixmap icon() const;

    QSize sizeHint() const override;

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void drawShadow(QPainter& painter, const QRectF& rect);
    void drawBackground(QPainter& painter, const QRectF& rect);
    void drawIcon(QPainter& painter, const QRectF& rect);
    void drawTexts(QPainter& painter, const QRectF& rect);
    void drawHeatTag(QPainter& painter, const QRectF& rect);
    void setUrl(const QString& url);
    QString url() const;

private:
    QNetworkAccessManager* m_netManager = nullptr;

    QString m_siteName;
    QString m_description;
    QString m_heatText;
    QPixmap m_icon;
    QString m_url;

    bool m_hovered = false;
    bool m_pressed = false;
};