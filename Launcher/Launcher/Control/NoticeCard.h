#pragma once

#include <QWidget>
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

class NoticeCard : public QWidget
{
    Q_OBJECT

public:
    explicit NoticeCard(QWidget* parent = nullptr);
    explicit NoticeCard(const QString& content, QWidget* parent = nullptr);

    void setContent(const QString& content);
    QString content() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_content;
};