#pragma once
#ifndef INFOCARD_H
#define INFOCARD_H

#include <QFrame>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

class InfoCard : public QFrame
{
    Q_OBJECT

public:
    explicit InfoCard(const QString& title,
        const QString& content = QString(),
        QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setContent(const QString& content);

    QString title() const;
    QString content() const;

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_contentLabel = nullptr;
    QVBoxLayout* m_layout = nullptr;
};

#endif // INFOCARD_H