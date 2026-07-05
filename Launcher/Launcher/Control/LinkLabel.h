#pragma once
#include <QWidget>
#include <QString>

class LinkLabel : public QWidget
{
    Q_OBJECT
public:
    explicit LinkLabel(const QString& text = QString(),
        const QString& url = QString(),
        QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

    void setUrl(const QString& url);
    QString url() const;

    void setTextColor(const QColor& color);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString m_text;
    QString m_url;
    QColor m_textColor = QColor(0, 120, 215); // 蓝色
    bool m_hovered = false;
};