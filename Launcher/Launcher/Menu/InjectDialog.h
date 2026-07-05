#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QTimer>

#include "../Control/CustomButton.h"
#include "../Control/CapsuleCheckBox.h"

struct InjectOptions {
    bool vida = false;
    bool mod = false;
};

class InjectDialog : public QWidget
{
    Q_OBJECT
public:
    explicit InjectDialog(QWidget* parent = nullptr);

    void showWithAnimation();
    void fadeOutAndClose();
    void setOptions(const InjectOptions& options);
    InjectOptions getOptions() const;

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
    CapsuleCheckBox* m_dllCheck = nullptr;
    CapsuleCheckBox* m_modCheck = nullptr;

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