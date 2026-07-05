#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QFileDialog>
#include <QParallelAnimationGroup>
#include <QTimer>

#include "../Control/CustomButton.h"
#include "../Control/CustomLineEdit.h"

class PathDialog : public QWidget
{
    Q_OBJECT
public:
    explicit PathDialog(QWidget* parent = nullptr);

    void showWithAnimation();
    void fadeOutAndClose();
    void setPath(const QString& path, const QString& steampath);  // 设置输入框内容
    QString getPath() const;                                  // 获取输入框内容

signals:
    void confirmed();
    void canceled();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void startOpenAnimation();
    void startCloseAnimation();
    void updatePanelLayout();

private:
    QWidget* m_topMask;
    QWidget* m_blockMask;
    QWidget* m_panel;

    CustomButton* btnOk;
    CustomButton* btnCancel;
    CustomLineEdit* m_pathEdit = nullptr;

    QString m_execPath;  // 程序路径
    QString m_steamPath; // steam路径

    QTimer* m_frameTimer = nullptr;
    QElapsedTimer m_elapsedTimer;

    bool m_closing = false;
    bool m_animating = false;
    bool m_opening = false;

    int m_animDuration = 180;

    qreal m_progress = 0.0;
    qreal m_maskOpacity = 0.0;
    int m_panelOffsetY = 0;

    QPixmap m_panelSnapshot;
    qreal m_panelOpacity = 0.0;
    bool m_useSnapshot = false;
};