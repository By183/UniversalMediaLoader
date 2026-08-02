#include "ToggleSwitch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFontMetrics>

ToggleSwitch::ToggleSwitch(const QString &text, QWidget *parent)
    : QAbstractButton(parent), m_handlePosition(1.0), m_textColor(Qt::white), m_anim(nullptr)
{
    m_anim = new QPropertyAnimation(this, "handlePosition", this);
    m_anim->setDuration(160);
    m_anim->setEasingCurve(QEasingCurve::InOutQuad);

    setText(text);
    setCheckable(true);
    setChecked(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ToggleSwitch::setHandlePosition(qreal pos) {
    m_handlePosition = pos;
    update();
}

void ToggleSwitch::setTextColor(const QColor &color) {
    m_textColor = color;
    update();
}

QSize ToggleSwitch::sizeHint() const {
    int trackW = 48;
    int trackH = 26;
    if (text().isEmpty()) {
        return QSize(trackW, trackH);
    }
    QFontMetrics fm(font());
    int textW = fm.horizontalAdvance(text());
    return QSize(trackW + 10 + textW, qMax(trackH, fm.height()));
}

bool ToggleSwitch::hitButton(const QPoint &pos) const {
    int trackH = 26;
    int trackY = (height() - trackH) / 2;
    QRect trackRect(0, trackY, 48, trackH);
    return trackRect.contains(pos);
}

void ToggleSwitch::mousePressEvent(QMouseEvent *event) {
    QAbstractButton::mousePressEvent(event);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *event) {
    QAbstractButton::mouseReleaseEvent(event);
}

void ToggleSwitch::nextCheckState() {
    QAbstractButton::nextCheckState();
    checkStateSet();
}

void ToggleSwitch::checkStateSet() {
    QAbstractButton::checkStateSet();
    if (!m_anim) return;

    m_anim->stop();
    m_anim->setStartValue(m_handlePosition);
    m_anim->setEndValue(isChecked() ? 1.0 : 0.0);
    m_anim->start();
}

void ToggleSwitch::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int trackW = 48;
    int trackH = 26;
    int trackY = (height() - trackH) / 2;

    QRectF trackRect(0, trackY, trackW, trackH);

    QColor offColor(107, 114, 128);
    QColor onColor(59, 130, 246);

    int r = qRound(offColor.red() + (onColor.red() - offColor.red()) * m_handlePosition);
    int g = qRound(offColor.green() + (onColor.green() - offColor.green()) * m_handlePosition);
    int b = qRound(offColor.blue() + (onColor.blue() - offColor.blue()) * m_handlePosition);
    QColor currentTrackColor(r, g, b);

    painter.setPen(Qt::NoPen);
    painter.setBrush(currentTrackColor);
    painter.drawRoundedRect(trackRect, trackH / 2.0, trackH / 2.0);

    int handleMargin = 3;
    int handleSize = trackH - 2 * handleMargin; // 20px
    qreal minX = handleMargin;
    qreal maxX = trackW - handleMargin - handleSize;
    qreal currentHandleX = minX + (maxX - minX) * m_handlePosition;
    QRectF handleRect(currentHandleX, trackY + handleMargin, handleSize, handleSize);

    painter.setBrush(Qt::white);
    painter.drawEllipse(handleRect);

    // ── Sun / Moon icon crossfade inside handle circle ──
    QFont iconFont = font();
    iconFont.setPointSize(10);
    painter.setFont(iconFont);

    if (m_handlePosition < 0.99) {
        painter.setOpacity(1.0 - m_handlePosition);
        painter.setPen(QColor(234, 179, 8)); // Golden sun
        painter.drawText(handleRect, Qt::AlignCenter, "☀");
    }

    if (m_handlePosition > 0.01) {
        painter.setOpacity(m_handlePosition);
        painter.setPen(QColor(99, 102, 241)); // Deep blue moon
        painter.drawText(handleRect, Qt::AlignCenter, "🌙");
    }

    painter.setOpacity(1.0);
    if (!text().isEmpty()) {
        painter.setPen(m_textColor);
        QRectF textRect(trackW + 10, 0, width() - trackW - 10, height());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
    }
}
