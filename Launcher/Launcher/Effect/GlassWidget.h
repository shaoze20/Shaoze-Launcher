#pragma once

#include <QWidget>
#include <QImage>

class GlassWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlassWidget(QWidget* parent = nullptr);

    void setBackground(const QImage& img);
    void setBlurRadius(int radius);
    void setGlassColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage m_sourceImage;
    QImage m_blurImage;

    int m_blurRadius = 10;   // 默认模糊强度

    QColor m_glassColor = QColor(20, 20, 20, 80);
    QImage blurImage(const QImage& src, int radius);
};