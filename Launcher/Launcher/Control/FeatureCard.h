#pragma once
#include <QWidget>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QGraphicsDropShadowEffect>
#include <QWindow>

class FeatureCard : public QWidget
{
    Q_OBJECT

public:
    explicit FeatureCard(QWidget* parent = nullptr);

    void setName(const QString& name);
    void setDescription(const QString& description);
    void setIcon(const QPixmap& icon);
    void setImageUrl(const QString& url);

    QString name() const { return m_name; }
    QString description() const { return m_description; }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onImageDownloaded(QNetworkReply* reply);

private:
    void updateScaledPixmaps();
    QPainterPath roundedPath(const QRectF& rect, qreal radius) const;
    void updateShadowEffect();
    void startShadowAnimation(qreal endBlur, int endAlpha);

private:
    QGraphicsDropShadowEffect* m_shadowEffect = nullptr;

    QString m_name = QStringLiteral("名字");
    QString m_description = QStringLiteral("这里是描述文字");
    QString m_imageUrl;

    QPixmap m_icon;
    QPixmap m_iconScaled;

    QPixmap m_imageOriginal;
    QPixmap m_imageScaled;

    QNetworkAccessManager* m_net = nullptr;

    bool m_hovered = false;
    bool m_pressed = false;

    QSize m_lastImageScaledTarget;
    QSize m_lastIconScaledTarget;
};