#include "SidebarWidget.h"
#include <QDir>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QPainter>
#include <QPolygonF>
#include <QPainterPath>

class PlayTriangleIcon : public QWidget {
public:
    explicit PlayTriangleIcon(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(26, 26);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPolygonF triangle;
        triangle << QPointF(7.5, 5.5)
                 << QPointF(19.5, 13.0)
                 << QPointF(7.5, 20.5);

        painter.setBrush(QColor("#FF0000"));
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }
};

class GamepadIcon : public QWidget {
public:
    explicit GamepadIcon(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(26, 26);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#9146FF"));

        // Main body - rounded rectangle
        QPainterPath body;
        body.addRoundedRect(3, 7, 20, 12, 4, 4);
        painter.drawPath(body);

        // Left grip
        QPainterPath leftGrip;
        leftGrip.addRoundedRect(4, 14, 5, 7, 2, 2);
        painter.drawPath(leftGrip);

        // Right grip
        QPainterPath rightGrip;
        rightGrip.addRoundedRect(17, 14, 5, 7, 2, 2);
        painter.drawPath(rightGrip);

        // D-pad (left side) - cross shape
        painter.setBrush(QColor("#1a1a2e"));
        painter.drawRect(7, 11, 2, 6);  // vertical bar
        painter.drawRect(5, 13, 6, 2);  // horizontal bar

        // Buttons (right side) - two small circles
        painter.drawEllipse(QPointF(18, 11), 1.3, 1.3);
        painter.drawEllipse(QPointF(21, 13), 1.3, 1.3);
    }
};

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("SidebarWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(210);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 24, 16, 16);
    layout->setSpacing(6);

    // 1. Logo row & Subtitle
    QLabel *logoLabel = new QLabel("🎬 UML", this);
    QFont logoFont = logoLabel->font();
    logoFont.setPointSize(24);
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("Universal Media\nLoader", this);
    subtitleLabel->setProperty("class", "MutedLabel");
    QFont subFont = subtitleLabel->font();
    subFont.setPointSize(9);
    subtitleLabel->setFont(subFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    layout->addWidget(logoLabel);
    layout->addWidget(subtitleLabel);
    layout->addSpacing(8);

    // 2. Horizontal separator line
    layout->addWidget(createSeparator());
    layout->addSpacing(8);

    // 3. Section label "Платформы"
    QLabel *platformsHeader = new QLabel("Платформы", this);
    QFont sectionFont = platformsHeader->font();
    sectionFont.setPointSize(14);
    sectionFont.setBold(true);
    platformsHeader->setFont(sectionFont);
    layout->addWidget(platformsHeader);

    // 4. Three platform rows
    // type: 0=emoji, 1=PlayTriangle, 2=Gamepad
    auto addPlatform = [this, layout](const QString &icon, const QString &name, const QString &hexColor, int iconType = 0) {
        QWidget *rowWidget = new QWidget(this);
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(4, 2, 0, 2);
        rowLayout->setSpacing(8);

        if (iconType == 1) {
            PlayTriangleIcon *triIcon = new PlayTriangleIcon(rowWidget);
            rowLayout->addWidget(triIcon);
        } else if (iconType == 2) {
            GamepadIcon *gpIcon = new GamepadIcon(rowWidget);
            rowLayout->addWidget(gpIcon);
        } else {
            QLabel *iconLabel = new QLabel(icon, rowWidget);
            iconLabel->setFixedWidth(26);
            iconLabel->setAlignment(Qt::AlignCenter);
            QFont iconFont = iconLabel->font();
            iconFont.setPointSize(13);
            iconFont.setBold(true);
            iconLabel->setFont(iconFont);
            iconLabel->setStyleSheet(QString("color: %1;").arg(hexColor));
            rowLayout->addWidget(iconLabel);
        }

        QLabel *nameLabel = new QLabel(name, rowWidget);
        QFont nameFont = nameLabel->font();
        nameFont.setPointSize(13);
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);
        nameLabel->setStyleSheet(QString("color: %1;").arg(hexColor));
        rowLayout->addWidget(nameLabel);
        rowLayout->addStretch();

        layout->addWidget(rowWidget);
    };

    addPlatform("", "YouTube", "#FF0000", 1);
    addPlatform("", "Twitch", "#9146FF", 2);
    addPlatform("🎵", "TikTok", "#00E676", 0);

    layout->addSpacing(8);
    layout->addWidget(createSeparator());
    layout->addSpacing(8);

    // 5. Output directory section
    QLabel *dirHeader = new QLabel("Путь сохранения:", this);
    QFont dirHeaderFont = dirHeader->font();
    dirHeaderFont.setPointSize(10);
    dirHeaderFont.setBold(true);
    dirHeader->setFont(dirHeaderFont);
    layout->addWidget(dirHeader);

    m_pathLabel = new QLabel(this);
    m_pathLabel->setProperty("class", "PathLabel");
    m_pathLabel->setWordWrap(true);
    QFont pathFont = m_pathLabel->font();
    pathFont.setPointSize(9);
    m_pathLabel->setFont(pathFont);
    layout->addWidget(m_pathLabel);

    layout->addSpacing(4);

    QPushButton *btnSelectFolder = new QPushButton("📁  Выбрать папку", this);
    btnSelectFolder->setProperty("class", "SidebarButton");
    btnSelectFolder->setCursor(Qt::PointingHandCursor);
    layout->addWidget(btnSelectFolder);

    QPushButton *btnOpenFolder = new QPushButton("📂  Открыть папку", this);
    btnOpenFolder->setProperty("class", "SidebarButton");
    btnOpenFolder->setCursor(Qt::PointingHandCursor);
    layout->addWidget(btnOpenFolder);

    layout->addSpacing(4);

    m_btnUpdateEngine = new QPushButton("🔄  Обновить движок", this);
    m_btnUpdateEngine->setProperty("class", "SidebarButton");
    m_btnUpdateEngine->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_btnUpdateEngine);

    layout->addSpacing(8);
    layout->addWidget(createSeparator());
    layout->addSpacing(8);

    // 6. Theme toggle section
    QLabel *themeHeader = new QLabel("Тема оформления", this);
    QFont themeFont = themeHeader->font();
    themeFont.setPointSize(9);
    themeHeader->setFont(themeFont);
    themeHeader->setProperty("class", "MutedLabel");
    layout->addWidget(themeHeader);

    m_themeSwitch = new ToggleSwitch("Тёмная тема", this);
    layout->addWidget(m_themeSwitch);

    layout->addStretch();

    // 7. Version label at bottom
    QLabel *versionLabel = new QLabel("v2.0.0", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setProperty("class", "MutedLabel");
    QFont verFont = versionLabel->font();
    verFont.setPointSize(9);
    versionLabel->setFont(verFont);
    layout->addWidget(versionLabel);

    connect(btnSelectFolder, &QPushButton::clicked, this, &SidebarWidget::selectFolderRequested);
    connect(btnOpenFolder, &QPushButton::clicked, this, &SidebarWidget::openFolderRequested);
    connect(m_btnUpdateEngine, &QPushButton::clicked, this, &SidebarWidget::updateEngineRequested);
    connect(m_themeSwitch, &ToggleSwitch::toggled, this, &SidebarWidget::themeToggled);
}

void SidebarWidget::setSavePath(const QString &path) {
    m_savePath = path;
    if (m_pathLabel) {
        m_pathLabel->setText(shortenPath(QDir::toNativeSeparators(path)));
    }
}

void SidebarWidget::setUpdatingEngineState(bool updating) {
    if (m_btnUpdateEngine) {
        m_btnUpdateEngine->setEnabled(!updating);
        m_btnUpdateEngine->setText(updating ? "🔄  Обновление..." : "🔄  Обновить движок");
    }
}

void SidebarWidget::setThemeChecked(bool isDark) {
    if (m_themeSwitch) {
        QSignalBlocker blocker(m_themeSwitch);
        m_themeSwitch->setChecked(isDark);
    }
}

QFrame* SidebarWidget::createSeparator() {
    QFrame *frame = new QFrame(this);
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    frame->setObjectName("SidebarSeparator");
    return frame;
}

QString SidebarWidget::shortenPath(const QString &path, int maxLength) {
    if (path.length() <= maxLength) return path;
    return path.left(10) + "..." + path.right(maxLength - 13);
}
