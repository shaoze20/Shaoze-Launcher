#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>

#include "../Control/CapsuleCheckBox.h"
#include "../Globals/Globals.h"

class SettingCard : public QWidget
{
    Q_OBJECT

public:
    explicit SettingCard(QWidget* parent = nullptr);

    void setDescription(const QString& description);
    QString description() const;

    void setImage(const QPixmap& pixmap);
    QPixmap image() const;

    void setImageVisible(bool visible = true);
    bool isImageVisible() const;

    void setChecked(bool checked);
    bool isChecked() const;

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateCheckBoxGeometry();

private:
    QString m_description;
    QPixmap m_pixmap;
    bool m_imageVisible = true;

    CapsuleCheckBox* m_checkBox = nullptr;
};