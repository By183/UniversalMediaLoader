#include "MainWindow.h"
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QIcon>
#include <QMetaObject>
#include <QPixmap>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include "CommandBuilder.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_isDark(true),
      m_runner(new ProcessRunner(this)),
      m_updater(new EngineUpdater(this))
{
    setWindowTitle("Universal Media Loader by Timur");
    resize(1050, 620);
    setMinimumSize(950, 580);
    setWindowIcon(QIcon(":/icon.ico"));

    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("MainContentWidget");
    m_centralWidget->setAttribute(Qt::WA_StyledBackground, true);
    setCentralWidget(m_centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_sidebar = new SidebarWidget(m_centralWidget);
    mainLayout->addWidget(m_sidebar);

    m_mainPanel = new MainPanel(m_centralWidget);
    mainLayout->addWidget(m_mainPanel, 1);

    connect(m_sidebar, &SidebarWidget::themeToggled, this, &MainWindow::setTheme);
    connect(m_sidebar, &SidebarWidget::selectFolderRequested, this, &MainWindow::onSelectFolder);
    connect(m_sidebar, &SidebarWidget::openFolderRequested, this, &MainWindow::onOpenFolder);
    connect(m_sidebar, &SidebarWidget::updateEngineRequested, this, &MainWindow::onUpdateEngineRequested);

    connect(m_mainPanel, &MainPanel::downloadRequested, this, &MainWindow::onStartDownload);
    connect(m_mainPanel, &MainPanel::pauseRequested, this, &MainWindow::onPauseRequested);
    connect(m_mainPanel, &MainPanel::cancelRequested, this, &MainWindow::onCancelRequested);

    connect(m_runner, &ProcessRunner::pauseStateChanged, this, &MainWindow::onPauseStateChanged);
    connect(m_runner, &ProcessRunner::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_runner, &ProcessRunner::logOutputEmitted, this, &MainWindow::onLogEmitted);
    connect(m_runner, &ProcessRunner::finished, this, &MainWindow::onProcessFinished);

    connect(m_updater, &EngineUpdater::statusEmitted, m_mainPanel, &MainPanel::setStatus);
    connect(m_updater, &EngineUpdater::logEmitted, m_mainPanel, &MainPanel::appendLog);
    connect(m_updater, &EngineUpdater::finished, this, [this](bool success, const QString &message) {
        m_sidebar->setUpdatingEngineState(false);
        m_mainPanel->setStatus(message);
        m_mainPanel->appendLog(message);
    });

    loadConfig();
    setTheme(m_isDark);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_runner && m_runner->isRunning()) {
        m_runner->cancel();
    }
    QMainWindow::closeEvent(event);
}

QString MainWindow::getConfigPath() const {
    return QDir::homePath() + "/.uml_config.json";
}

void MainWindow::loadConfig() {
    QString path = getConfigPath();
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("output_dir")) {
                QString dir = obj.value("output_dir").toString();
                if (!dir.isEmpty()) {
                    m_sidebar->setSavePath(dir);
                }
            }
            if (obj.contains("is_dark")) {
                m_isDark = obj.value("is_dark").toBool();
            }
        }
    }
}

void MainWindow::saveConfig() {
    QJsonObject obj;
    obj["output_dir"] = m_sidebar->savePath();
    obj["is_dark"] = m_isDark;

    QString finalPath = getConfigPath();
    QString tempPath = finalPath + ".tmp";

    QFile tempFile(tempPath);
    if (tempFile.open(QFile::WriteOnly)) {
        tempFile.write(QJsonDocument(obj).toJson());
        tempFile.close();
        if (QFile::exists(finalPath)) {
            QFile::remove(finalPath);
        }
        QFile::rename(tempPath, finalPath);
    }
}

void MainWindow::onSelectFolder() {
    QString current = m_sidebar->savePath();
    if (current.isEmpty()) {
        current = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения", current);
    if (!dir.isEmpty()) {
        m_sidebar->setSavePath(dir);
        saveConfig();
        m_mainPanel->appendLog(QString("📁  Папка сохранения изменена на: %1").arg(dir));
    }
}

void MainWindow::onOpenFolder() {
    QString path = m_sidebar->savePath();
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::onUpdateEngineRequested() {
    if (m_runner->isRunning()) {
        m_mainPanel->setStatus("⚠  Нельзя обновлять движок во время скачивания!");
        return;
    }
    if (m_updater->isUpdating()) {
        m_mainPanel->setStatus("⚠  Обновление движка уже выполняется...");
        return;
    }
    m_sidebar->setUpdatingEngineState(true);
    m_mainPanel->clearLog();
    m_updater->startUpdate();
}

void MainWindow::onStartDownload(DownloadMode mode) {
    if (m_updater->isUpdating()) {
        m_mainPanel->setStatus("⚠  Идёт обновление движка, подождите...");
        return;
    }

    if (m_runner->isRunning()) {
        m_mainPanel->setStatus("⚠  Загрузка уже выполняется!");
        return;
    }

    m_lastStatusMessage.clear();
    m_mainPanel->clearLog();

    QString url = m_mainPanel->urlText();
    if (url.isEmpty()) {
        m_mainPanel->setStatus("⚠  Пожалуйста, вставьте ссылку на видео");
        return;
    }

    QUrl qurl(url);
    bool hasValidScheme = url.startsWith("http://", Qt::CaseInsensitive) || url.startsWith("https://", Qt::CaseInsensitive);
    if (!hasValidScheme || !qurl.isValid() || !url.contains(".")) {
        m_mainPanel->setStatus("⚠  Некорректная ссылка — проверьте, что вы вставили правильный адрес");
        return;
    }

    QString quality = m_mainPanel->selectedQuality();
    QString customName = m_mainPanel->customNameText();
    QString timeStart = m_mainPanel->timeStartText();
    QString timeEnd = m_mainPanel->timeEndText();
    QString outputDir = m_sidebar->savePath();

    if (outputDir.isEmpty()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    
    QDir targetDir(outputDir);
    if (!targetDir.exists() && !targetDir.mkpath(".")) {
        m_mainPanel->setStatus("❌  Папка сохранения недоступна — выберите другую папку");
        m_mainPanel->appendLog(QString("❌  Не удалось создать или получить доступ к папке: %1").arg(outputDir));
        return;
    }

    QStringList args = CommandBuilder::buildCommand(url, quality, mode, outputDir, customName, timeStart, timeEnd);

    m_mainPanel->setGuiDownloadingState(true);
    m_mainPanel->setPausedState(false);
    m_mainPanel->setProgress(0);

    QString modeStr = (mode == DownloadMode::Audio) ? "MP3" : ((mode == DownloadMode::Thumbnail) ? "Превью" : quality);
    m_mainPanel->setStatus(mode == DownloadMode::Thumbnail ? "🖼  Скачивание превью изображения..." : "🔍  Получение информации о видео...");
    m_mainPanel->appendLog(QString("▶  Запуск (%1): %2").arg(modeStr, url));
    if (mode != DownloadMode::Thumbnail && (!timeStart.isEmpty() || !timeEnd.isEmpty())) {
        m_mainPanel->appendLog(QString("⏱  Обрезка фрагмента: От %1 До %2")
                                   .arg(timeStart.isEmpty() ? "0" : timeStart)
                                   .arg(timeEnd.isEmpty() ? "конец" : timeEnd));
    }
    m_mainPanel->appendLog(QString("📂  Папка: %1").arg(outputDir));

    m_runner->startDownload(args, outputDir);
}

void MainWindow::onPauseRequested() {
    if (!m_runner->isRunning()) return;
    if (m_runner->isPaused()) {
        m_runner->resume();
        m_mainPanel->appendLog("▶  Загрузка возобновлена пользователем");
    } else {
        m_runner->pause();
        m_mainPanel->appendLog("⏸  Загрузка приостановлена пользователем");
    }
}

void MainWindow::onCancelRequested() {
    if (m_runner->isRunning() || m_runner->isPaused()) {
        m_mainPanel->appendLog("✕  Отмена загрузки пользователем...");
        m_runner->cancel();
    }
}

void MainWindow::onPauseStateChanged(bool isPaused) {
    m_mainPanel->setPausedState(isPaused);
}

void MainWindow::onProgressUpdated(const ParsedProgress &progress) {
    if (progress.isIndeterminate) {
        m_mainPanel->setProgress(0, true);
    } else if (progress.hasPercentage) {
        m_mainPanel->setProgress(qRound(progress.percentage), false);
    }
    if (!progress.statusMessage.isEmpty()) {
        m_lastStatusMessage = progress.statusMessage;
        m_mainPanel->setStatus(progress.statusMessage);
    }
}

void MainWindow::onLogEmitted(const QString &rawLine) {
    static QStringList keywords = {"[download]", "[Merger]", "ExtractAudio", "ERROR", "WARNING", "has already been downloaded", "[Fixup", "--download-sections", "[youtube:tab]", "Downloading video", "Downloading item", "[Thumbnails]"};
    bool important = false;
    for (const QString &kw : keywords) {
        if (rawLine.contains(kw)) {
            important = true;
            break;
        }
    }
    if (important) {
        m_mainPanel->appendLog(QString("  ▸ %1").arg(rawLine));
        if (rawLine.contains("ERROR", Qt::CaseInsensitive) && (rawLine.contains("410", Qt::CaseInsensitive) || rawLine.contains("Gone", Qt::CaseInsensitive))) {
            m_mainPanel->appendLog("  ▸ 🔒  Контент недоступен: Видео было удалено с сервера (HTTP Error 410: Gone)");
        }
    }
}

void MainWindow::onProcessFinished(bool success, int exitCode) {
    m_mainPanel->setGuiDownloadingState(false);
    m_mainPanel->setPausedState(false);
    if (exitCode == -1) {
        m_mainPanel->setProgress(0, false);
        m_mainPanel->setStatus("✕  Загрузка отменена");
        m_mainPanel->appendLog("✕  Загрузка отменена. Временные файлы удалены.");
    } else if (success) {
        m_mainPanel->setProgress(100, false);
        m_mainPanel->setStatus("✅  Загрузка завершена!");
        m_mainPanel->appendLog("✅  Загрузка завершена успешно!");
    } else {
        m_mainPanel->setProgress(0, false);
        if (!m_lastStatusMessage.isEmpty() && (m_lastStatusMessage.startsWith("🔒") || m_lastStatusMessage.startsWith("🔄") || m_lastStatusMessage.startsWith("❌") || m_lastStatusMessage.startsWith("⚠"))) {
            m_mainPanel->setStatus(m_lastStatusMessage);
        } else {
            m_mainPanel->setStatus("❌  Ошибка загрузки — подробности в логе ниже");
        }
        m_mainPanel->appendLog(QString("❌  Процесс завершился с ошибкой (код выхода %1)").arg(exitCode));
    }
}

void MainWindow::setTheme(bool isDark) {
    m_isDark = isDark;
    saveConfig();

    QString qssFile = isDark ? ":/styles/dark.qss" : ":/styles/light.qss";
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
        file.close();
    }
}
