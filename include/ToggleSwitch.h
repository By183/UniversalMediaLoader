#ifndef TOGGLESWITCH_H
#define TOGGLESWITCH_H

#include <QAbstractButton>
#include <QPropertyAnimation>
#include <QMouseEvent>

class ToggleSwitch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal handlePosition READ handlePosition WRITE setHandlePosition)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

public:
    explicit ToggleSwitch(const QString &text = "", QWidget *parent = nullptr);

    qreal handlePosition() const { return m_handlePosition; }
    void setHandlePosition(qreal pos);

    QColor textColor() const { return m_textColor; }
    void setTextColor(const QColor &color);

    QSize sizeHint() const override;

protected:
    bool hitButton(const QPoint &pos) const override;
    void nextCheckState() override;
    void paintEvent(QPaintEvent *event) override;
    void checkStateSet() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    qreal m_handlePosition = 1.0; // 0.0 (off) to 1.0 (on)
    QColor m_textColor = Qt::white;
    QPropertyAnimation *m_anim = nullptr;
};

#endif // TOGGLESWITCH_H
