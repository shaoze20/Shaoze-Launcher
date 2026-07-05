#pragma once
#include <QAbstractButton>
#include <QPainter>
#include <QPropertyAnimation>
#include <QFontMetrics>
#include <QEvent>
#include <QResizeEvent>
#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPainterPath>
#include "MaterialSpinner.h"

class StartButton : public QAbstractButton
{
    Q_OBJECT
        Q_PROPERTY(qreal textAnim READ textAnim WRITE setTextAnim)
        Q_PROPERTY(qreal textAlpha READ textAlpha WRITE setTextAlpha)
        Q_PROPERTY(qreal textShift READ textShift WRITE setTextShift)

public:
    explicit StartButton(QWidget* parent = nullptr)
        : QAbstractButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setCheckable(false);
        setMinimumSize(160, 56);

        m_spinner = new MaterialSpinner(QSize(30, 30), 5, QColor(124, 252, 0), this);
        m_spinner->hide();

        m_textAnim = 0.0;
        m_anim = new QPropertyAnimation(this, "textAnim", this);
        m_anim->setDuration(260);
        m_anim->setEasingCurve(QEasingCurve::InOutCubic);

        m_fadeOut = new QPropertyAnimation(this, "textAlpha", this);
        m_fadeOut->setDuration(120);
        m_fadeOut->setEasingCurve(QEasingCurve::InOutSine);

        m_fadeIn = new QPropertyAnimation(this, "textAlpha", this);
        m_fadeIn->setDuration(220);
        m_fadeIn->setEasingCurve(QEasingCurve::InOutSine);

        m_shiftAnim = new QPropertyAnimation(this, "textShift", this);
        m_shiftAnim->setDuration(220);
        m_shiftAnim->setEasingCurve(QEasingCurve::InOutCubic);

        m_scaleAndFadeIn = new QParallelAnimationGroup(this);
        m_scaleAndFadeIn->addAnimation(m_anim);      // 字号缩放
        m_scaleAndFadeIn->addAnimation(m_fadeIn);    // 淡入
        m_scaleAndFadeIn->addAnimation(m_shiftAnim); // 位移：并行执行

        m_textGroup = new QSequentialAnimationGroup(this);
        m_textGroup->addAnimation(m_fadeOut);
        m_textGroup->addAnimation(m_scaleAndFadeIn);

        connect(m_textGroup, &QSequentialAnimationGroup::finished, this, [this]() {
            setTextAlpha(1.0);
            setTextAnim(m_targetAnim);

            if (m_applySpinnerAfterAnim) {
                if (m_pendingSpinnerVisible) {
                    m_spinner->show();
                    m_spinner->raise();
                }
                else {
                    m_spinner->hide();
                }
                m_applySpinnerAfterAnim = false;
            }

            m_forceReserveSpinner = false;

            if (b_enabled) {
                setEnabled(!m_loading);
            }

            update();
            });

        m_normalPointSize = 16;
        m_normalBold = true;

        m_loadingPointSize = 13;
        m_loadingBold = true;

        setText(QStringLiteral("启动游戏"));
    }

    int calcStartX(int textW, bool reserveSpinner) const
    {
        const int spacing = 8;
        const int spW = reserveSpinner ? m_spinner->width() : 0;
        const int totalW = textW + (reserveSpinner ? (spacing + spW) : 0);
        return rect().center().x() - totalW / 2;
    }

    void setLoading(bool loading, bool enabled = true)
    {
        if (m_loading == loading) return;
        b_enabled = enabled;

        m_loading = loading;

        if (m_loading) {
            m_pendingSpinnerVisible = true;
            m_applySpinnerAfterAnim = true;
            m_forceReserveSpinner = false;

            animateToLoadingFont(true, true);
        }
        else {
            if (m_spinner) m_spinner->hide();

            m_forceReserveSpinner = false;
            m_applySpinnerAfterAnim = false;
            m_pendingSpinnerVisible = false;

            animateToLoadingFont(false, true);
        }

        update();
    }

    bool isLoading() const { return m_loading; }

    void setButtonText(const QString& t)
    {
        if (t == QAbstractButton::text() && !m_hasPendingText)
            return;

        m_pendingText = t;
        m_hasPendingText = true;

        if (m_textGroup && m_textGroup->state() != QAbstractAnimation::Running) {
            animateToLoadingFont(m_loading, false);
        }
    }

    qreal textAnim() const { return m_textAnim; }
    void setTextAnim(qreal v)
    {
        m_textAnim = qBound<qreal>(0.0, v, 1.0);
        update();
    }

    qreal textAlpha() const { return m_textAlpha; }
    void setTextAlpha(qreal a)
    {
        m_textAlpha = qBound<qreal>(0.0, a, 1.0);
        update();
    }

    qreal textShift() const { return m_textShift; }
    void setTextShift(qreal s)
    {
        m_textShift = s;
        update();
    }

protected:
    void resizeEvent(QResizeEvent* e) override
    {
        QAbstractButton::resizeEvent(e);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRect r = rect();
        const int radius = 27; // 圆角

        QColor bg(255, 215, 0);
        if (!isEnabled()) bg.setAlpha(180);
        else if (isDown()) bg = bg.darker(110);
        else if (underMouse()) bg = bg.lighter(105);

        QPainterPath path;
        path.addRoundedRect(r.adjusted(1, 1, -1, -1), radius, radius);
        p.fillPath(path, bg);

        QColor border(0, 0, 0, 40);
        p.setPen(QPen(border, 1));
        p.drawPath(path);

        QFont f = font();
        const int pt = currentAnimatedPointSize();
        f.setPointSize(pt);
        f.setBold(currentAnimatedBold());
        p.setFont(f);

        QColor textColor(0, 0, 0, 230);
        textColor.setAlphaF((230.0 / 255.0) * m_textAlpha);
        p.setPen(textColor);

        const int spacing = 8;
        const bool spinnerVisible = (m_spinner && m_spinner->isVisible());

        QFontMetrics fm(f);
        const QString t = text();
        const int textW = fm.horizontalAdvance(t);
        const int textH = fm.height();

        const bool reserveSpinner = m_forceReserveSpinner || m_pendingSpinnerVisible || spinnerVisible;
        const int spW = reserveSpinner ? m_spinner->width() : 0;
        const int spH = reserveSpinner ? m_spinner->height() : 0;

        const int totalW = textW + (reserveSpinner ? (spacing + spW) : 0);

        const int centerX = r.center().x();
        const int centerY = r.center().y();

        const int startX = centerX - totalW / 2;
        const int textX = startX + int(m_textShift);
        const int textY = centerY + (textH / 2) - fm.descent();

        p.drawText(textX, textY, t);

        if (spinnerVisible) {
            const int spX = startX + int(m_textShift) + textW + spacing;
            const int spY = centerY - spH / 2;
            m_spinner->move(spX, spY);
            m_spinner->raise();
        }
    }

private:
    void animateToLoadingFont(bool toLoading, bool allowShift)
    {
        if (!m_textGroup) return;

        const qreal newTarget = toLoading ? 1.0 : 0.0;
        if (qFuzzyCompare(m_targetAnim, newTarget) && m_textGroup->state() == QAbstractAnimation::Running)
            return;

        m_targetAnim = newTarget;

        if (m_textGroup->state() == QAbstractAnimation::Running)
            m_textGroup->stop();

        const qreal old = m_textAnim;
        const qreal target = m_targetAnim;

        m_fadeOut->setDuration(120);
        m_fadeIn->setDuration(220);

        m_fadeOut->setStartValue(m_textAlpha);
        m_fadeOut->setEndValue(0.0);

        m_fadeIn->setStartValue(0.0);
        m_fadeIn->setEndValue(1.0);

        if (!allowShift) {
            m_anim->setStartValue(old);
            m_anim->setEndValue(old);
        }
        else {
            m_anim->setStartValue(old);
            m_anim->setEndValue(old);
        }

        disconnect(m_fadeOut, nullptr, this, nullptr);
        connect(m_fadeOut, &QPropertyAnimation::finished, this, [this, old, target, allowShift]() {
            if (m_hasPendingText) {
                QAbstractButton::setText(m_pendingText);
                m_hasPendingText = false;
            }

            if (allowShift) {
                setTextAnim(old);
                m_anim->setStartValue(old);
                m_anim->setEndValue(target);
            }
            });

        if (!allowShift) {
            m_shiftAnim->setStartValue(m_textShift);
            m_shiftAnim->setEndValue(m_textShift);
            m_textGroup->start();
            return;
        }

        const bool willShowSpinner = m_pendingSpinnerVisible;

        QFont f = font();
        f.setPointSize(currentAnimatedPointSize());
        f.setBold(currentAnimatedBold());
        QFontMetrics fm(f);
        const int textW = fm.horizontalAdvance(text());

        qreal startShift = m_textShift;

        if (willShowSpinner) {
            if (!m_spinner->isVisible()) {
                const qreal half = (m_spinner->width() + 8) / 2.0;
                startShift = half;
                setTextShift(startShift);
            }

            m_shiftAnim->setStartValue(startShift);
            m_shiftAnim->setEndValue(0.0);

            m_textGroup->start();
            return;
        }

        const int startX_withSpinner = calcStartX(textW, true);
        const int startX_withoutSpinner = calcStartX(textW, false);
        const int delta = startX_withSpinner - startX_withoutSpinner;

        startShift = m_textShift + delta;
        setTextShift(startShift);

        m_fadeOut->setDuration(0);
        m_fadeOut->setStartValue(1.0);
        m_fadeOut->setEndValue(1.0);

        m_fadeIn->setDuration(0);
        m_fadeIn->setStartValue(1.0);
        m_fadeIn->setEndValue(1.0);

        setTextAnim(old);
        m_anim->setStartValue(old);
        m_anim->setEndValue(target);

        m_shiftAnim->setStartValue(startShift);
        m_shiftAnim->setEndValue(0.0);

        m_textGroup->start();
    }

    int currentAnimatedPointSize() const
    {
        const qreal a = m_textAnim;
        const qreal v = m_normalPointSize + (m_loadingPointSize - m_normalPointSize) * a;
        return qRound(v);
    }

    bool currentAnimatedBold() const
    {
        return (m_textAnim >= 0.5) ? m_loadingBold : m_normalBold;
    }

private:
    MaterialSpinner* m_spinner = nullptr;
    QPropertyAnimation* m_anim = nullptr;

    QSequentialAnimationGroup* m_textGroup = nullptr;
    QPropertyAnimation* m_fadeOut = nullptr;
    QParallelAnimationGroup* m_scaleAndFadeIn = nullptr;
    QPropertyAnimation* m_fadeIn = nullptr;

    qreal m_textAlpha = 1.0;
    qreal m_targetAnim = 0.0;

    bool m_loading = false;
    qreal m_textAnim = 0.0;

    int  m_normalPointSize = 16;
    bool m_normalBold = true;

    int  m_loadingPointSize = 13;
    bool m_loadingBold = true;

    QString m_pendingText;
    bool m_hasPendingText = false;

    bool m_pendingSpinnerVisible = false;
    bool m_applySpinnerAfterAnim = false;
    bool m_forceReserveSpinner = false;

    QPropertyAnimation* m_shiftAnim = nullptr;
    qreal m_textShift = 0.0;

    bool b_enabled = true;
};