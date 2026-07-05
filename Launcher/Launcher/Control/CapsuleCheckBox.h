#pragma once

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>

class CapsuleCheckBox : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(qreal offset READ offset WRITE setOffset)

public:
    explicit CapsuleCheckBox(const QString& text = "", QWidget* parent = nullptr);

    void setText(const QString& text);

    bool isChecked() const;
    void setChecked(bool checked);

    qreal offset() const;
    void setOffset(qreal value);

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    QString m_text;
    bool m_checked = false;
    qreal m_offset = 0.0;

    QTimer* m_frameTimer = nullptr;
    QElapsedTimer m_elapsedTimer;

    qreal m_animStart = 0.0;
    qreal m_animEnd = 0.0;
    int m_animDuration = 140;
    bool m_animating = false;
};