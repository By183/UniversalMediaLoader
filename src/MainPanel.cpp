#include "MainPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QDebug>

MainPanel::MainPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("MainPanelWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // 1. App Title Row
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *headerIcon = new QLabel("🎬", this);
    QFont iconFont = headerIcon->font();
    iconFont.setPointSize(24);
    headerIcon->setFont(iconFont);

    QVBoxLayout *titleTextLayout = new QVBoxLayout();
    QLabel *titleLabel = new QLabel("Universal Media Loader", this);
    titleLabel->setObjectName("appTitleLabel");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    QLabel *subtitleLabel = new QLabel("YouTube • Twitch • TikTok", this);
    subtitleLabel->setObjectName("appSubtitleLabel");
    QFont subFont = subtitleLabel->font();
    subFont.setPointSize(10);
    subtitleLabel->setFont(subFont);

    titleTextLayout->addWidget(titleLabel);
    titleTextLayout->addWidget(subtitleLabel);

    headerLayout->addStretch();
    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(titleTextLayout);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(8);

    // 2. URL Input Row
    QHBoxLayout *urlLayout = new QHBoxLayout();
    urlLayout->setSpacing(8);

    m_urlInput = new QLineEdit(this);
    m_urlInput->setObjectName("UrlInput");
    m_urlInput->setPlaceholderText("Вставь ссылку (Twitch, YT, TikTok...)");
    m_urlInput->setFixedHeight(44);
    m_urlInput->setClearButtonEnabled(true);
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainPanel::parseAndSetTimestampFromUrl);

    m_pasteBtn = new QPushButton("📋 Вставить", this);
    m_pasteBtn->setObjectName("PasteButton");
    m_pasteBtn->setFixedHeight(44);
    m_pasteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_pasteBtn, &QPushButton::clicked, this, &MainPanel::onPasteClicked);

    urlLayout->addWidget(m_urlInput, 1);
    urlLayout->addWidget(m_pasteBtn);

    mainLayout->addLayout(urlLayout);

    // 3. Custom Name Input
    m_nameInput = new QLineEdit(this);
    m_nameInput->setObjectName("CustomNameInput");
    m_nameInput->setPlaceholderText("Кастомное название (необязательно)");
    m_nameInput->setFixedHeight(42);
    mainLayout->addWidget(m_nameInput);

    // 4. Quality & Thumbnail Button Layout
    QHBoxLayout *qaLayout = new QHBoxLayout();
    qaLayout->setSpacing(12);

    m_qualityCombo = new QComboBox(this);
    m_qualityCombo->setObjectName("QualityComboBox");
    m_qualityCombo->addItems({"1080p", "720p", "480p", "360p"});
    m_qualityCombo->setFixedHeight(44);
    m_qualityCombo->setCursor(Qt::PointingHandCursor);

    m_thumbnailBtn = new QPushButton("🖼 Скачать превью", this);
    m_thumbnailBtn->setObjectName("ThumbnailButton");
    m_thumbnailBtn->setFixedHeight(44);
    m_thumbnailBtn->setCursor(Qt::PointingHandCursor);
    connect(m_thumbnailBtn, &QPushButton::clicked, this, [this]() {
        emit downloadRequested(DownloadMode::Thumbnail);
    });

    qaLayout->addWidget(m_qualityCombo, 1);
    qaLayout->addWidget(m_thumbnailBtn, 2);

    m_audioOnlyBtn = new QPushButton("🎵 Скачать звук (MP3)", this);
    m_audioOnlyBtn->setObjectName("AudioOnlyButton");
    m_audioOnlyBtn->setFixedHeight(44);
    m_audioOnlyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_audioOnlyBtn, &QPushButton::clicked, this, [this]() {
        emit downloadRequested(DownloadMode::Audio);
    });
    qaLayout->addWidget(m_audioOnlyBtn, 2);

    mainLayout->addLayout(qaLayout);

    // 5. Main Action Buttons Row
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    m_downloadBtn = new QPushButton("⬇  Скачать Видео", this);
    m_downloadBtn->setObjectName("DownloadVideoButton");
    m_downloadBtn->setFixedHeight(48);
    m_downloadBtn->setCursor(Qt::PointingHandCursor);
    connect(m_downloadBtn, &QPushButton::clicked, this, [this]() {
        emit downloadRequested(DownloadMode::Video);
    });
    actionLayout->addWidget(m_downloadBtn, 3);

    m_pauseBtn = new QPushButton("⏸ Пауза", this);
    m_pauseBtn->setObjectName("PauseButton");
    m_pauseBtn->setFixedHeight(48);
    m_pauseBtn->setCursor(Qt::PointingHandCursor);
    m_pauseBtn->setEnabled(false);
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() { emit pauseRequested(); });
    actionLayout->addWidget(m_pauseBtn, 1);

    m_cancelBtn = new QPushButton("✕ Отмена", this);
    m_cancelBtn->setObjectName("CancelButton");
    m_cancelBtn->setFixedHeight(48);
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setEnabled(false);
    connect(m_cancelBtn, &QPushButton::clicked, this, [this]() { emit cancelRequested(); });
    actionLayout->addWidget(m_cancelBtn, 1);

    mainLayout->addLayout(actionLayout);

    // 6. Time cropping row
    m_cropCheckBox = new QCheckBox("⏱ Обрезка по времени (скачать выборочный фрагмент)", this);
    m_cropCheckBox->setObjectName("timeCropCheckbox");
    m_cropCheckBox->setCursor(Qt::PointingHandCursor);
    m_cropCheckBox->setChecked(false);
    connect(m_cropCheckBox, &QCheckBox::toggled, this, &MainPanel::onCropToggled);
    mainLayout->addWidget(m_cropCheckBox);

    m_cropContainer = new QWidget(this);
    QVBoxLayout *cropBoxContainerLayout = new QVBoxLayout(m_cropContainer);
    cropBoxContainerLayout->setContentsMargins(0, 0, 0, 0);
    cropBoxContainerLayout->setSpacing(4);

    QHBoxLayout *cropLayout = new QHBoxLayout();
    cropLayout->setSpacing(16);

    QHBoxLayout *startPairLayout = new QHBoxLayout();
    startPairLayout->setSpacing(4);
    QLabel *startLabel = new QLabel("От:", m_cropContainer);
    m_timeStartInput = new QLineEdit(m_cropContainer);
    m_timeStartInput->setObjectName("timeStartInput");
    m_timeStartInput->setPlaceholderText("мин или чч:мм:сс");
    m_timeStartInput->setToolTip("Введите минуты (например 50) или время в формате чч:мм:сс (например 1:20:00)");
    m_timeStartInput->setFixedHeight(38);
    startPairLayout->addWidget(startLabel);
    startPairLayout->addWidget(m_timeStartInput, 1);

    QHBoxLayout *endPairLayout = new QHBoxLayout();
    endPairLayout->setSpacing(4);
    QLabel *endLabel = new QLabel("До:", m_cropContainer);
    m_timeEndInput = new QLineEdit(m_cropContainer);
    m_timeEndInput->setObjectName("timeEndInput");
    m_timeEndInput->setPlaceholderText("мин или чч:мм:сс");
    m_timeEndInput->setToolTip("Введите минуты (например 70) или время в формате чч:мм:сс (необязательно)");
    m_timeEndInput->setFixedHeight(38);
    endPairLayout->addWidget(endLabel);
    endPairLayout->addWidget(m_timeEndInput, 1);

    cropLayout->addLayout(startPairLayout, 1);
    cropLayout->addLayout(endPairLayout, 1);
    cropBoxContainerLayout->addLayout(cropLayout);

    m_cropContainer->setVisible(false);
    mainLayout->addWidget(m_cropContainer);

    // 7. Status & Progress Bar Row
    m_statusLabel = new QLabel("Готов к работе", this);
    m_statusLabel->setObjectName("StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("MainProgressBar");
    m_progressBar->setFixedHeight(12);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);

    // 8. Log Area
    m_logEdit = new QPlainTextEdit(this);
    m_logEdit->setObjectName("LogArea");
    m_logEdit->setReadOnly(true);
    m_logEdit->setMinimumHeight(140);
    mainLayout->addWidget(m_logEdit, 1);
}

void MainPanel::onPasteClicked() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        m_urlInput->setText(clipboard->text().trimmed());
    }
}

void MainPanel::onCropToggled(bool checked) {
    m_cropContainer->setVisible(checked);
}

void MainPanel::parseAndSetTimestampFromUrl(const QString &urlStr) {
    if (urlStr.isEmpty()) return;

    static QRegularExpression rxT(R"([?&](?:t|start)=([0-9hms]+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = rxT.match(urlStr);
    if (match.hasMatch()) {
        QString timeRaw = match.captured(1);

        static QRegularExpression rxHMS(R"((?:(\d+)h)?(?:(\d+)m)?(?:(\d+)s)?)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch hmsMatch = rxHMS.match(timeRaw);

        int totalSeconds = 0;
        if (hmsMatch.hasMatch() && (hmsMatch.captured(1).length() > 0 || hmsMatch.captured(2).length() > 0 || hmsMatch.captured(3).length() > 0)) {
            int h = hmsMatch.captured(1).toInt();
            int m = hmsMatch.captured(2).toInt();
            int s = hmsMatch.captured(3).toInt();
            totalSeconds = h * 3600 + m * 60 + s;
        } else {
            totalSeconds = timeRaw.toInt();
        }

        if (totalSeconds > 0) {
            int hours = totalSeconds / 3600;
            int mins = (totalSeconds % 3600) / 60;
            int secs = totalSeconds % 60;

            QString formatted;
            if (hours > 0) {
                formatted = QString::asprintf("%02d:%02d:%02d", hours, mins, secs);
            } else {
                formatted = QString::asprintf("%02d:%02d", mins, secs);
            }
            m_cropCheckBox->setChecked(true);
            m_timeStartInput->setText(formatted);
        }
    }
}

QString MainPanel::urlText() const {
    return m_urlInput->text().trimmed();
}

void MainPanel::setUrlText(const QString &url) {
    m_urlInput->setText(url);
}

QString MainPanel::customNameText() const {
    return m_nameInput->text().trimmed();
}

QString MainPanel::selectedQuality() const {
    return m_qualityCombo->currentText();
}

QString MainPanel::timeStartText() const {
    return m_cropCheckBox->isChecked() ? m_timeStartInput->text().trimmed() : QString();
}

QString MainPanel::timeEndText() const {
    return m_cropCheckBox->isChecked() ? m_timeEndInput->text().trimmed() : QString();
}

void MainPanel::setCropTime(const QString &start, const QString &end) {
    m_cropCheckBox->setChecked(true);
    m_timeStartInput->setText(start);
    m_timeEndInput->setText(end);
}

void MainPanel::setStatus(const QString &status) {
    m_statusLabel->setText(status);
}

void MainPanel::setProgress(int percent, bool isIndeterminate) {
    if (isIndeterminate) {
        m_progressBar->setRange(0, 0);
    } else {
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(percent);
    }
}

void MainPanel::setGuiDownloadingState(bool downloading) {
    m_urlInput->setEnabled(!downloading);
    m_nameInput->setEnabled(!downloading);
    m_qualityCombo->setEnabled(!downloading);
    m_thumbnailBtn->setEnabled(!downloading);
    m_audioOnlyBtn->setEnabled(!downloading);
    m_downloadBtn->setEnabled(!downloading);
    m_cropCheckBox->setEnabled(!downloading);
    m_timeStartInput->setEnabled(!downloading);
    m_timeEndInput->setEnabled(!downloading);
    m_pasteBtn->setEnabled(!downloading);

    m_pauseBtn->setEnabled(downloading);
    m_cancelBtn->setEnabled(downloading);

    if (!downloading) {
        m_pauseBtn->setText("⏸ Пауза");
    }
}

void MainPanel::setPausedState(bool isPaused) {
    if (isPaused) {
        m_pauseBtn->setText("▶ Продолжить");
    } else {
        m_pauseBtn->setText("⏸ Пауза");
    }
}

void MainPanel::appendLog(const QString &message) {
    m_logEdit->appendPlainText(message);
}

void MainPanel::clearLog() {
    m_logEdit->clear();
}
