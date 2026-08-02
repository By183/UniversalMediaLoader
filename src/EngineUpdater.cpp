#include "EngineUpdater.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QUrl>

EngineUpdater::EngineUpdater(QObject *parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)), m_reply(nullptr), m_file(nullptr), m_isUpdating(false)
{
}

EngineUpdater::~EngineUpdater() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

void EngineUpdater::startUpdate() {
    if (m_isUpdating) return;

    QString appDir = QCoreApplication::applicationDirPath();
    m_targetFilePath = QDir(appDir).filePath("yt-dlp.exe");
    m_tempFilePath = QDir(appDir).filePath("yt-dlp.exe.download");

    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }

    m_file = new QFile(m_tempFilePath, this);
    if (!m_file->open(QIODevice::WriteOnly)) {
        emit finished(false, "Не удалось создать временный файл для скачивания.");
        delete m_file;
        m_file = nullptr;
        return;
    }

    m_isUpdating = true;
    emit logEmitted("🔄  Запуск обновления движка (yt-dlp)...");
    emit statusEmitted("🔄  Скачивание yt-dlp.exe...");

    QUrl url("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe");
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(15000); // 15 seconds network timeout

    m_reply = m_nam->get(request);

    connect(m_reply, &QNetworkReply::downloadProgress, this, &EngineUpdater::onDownloadProgress);
    connect(m_reply, &QNetworkReply::readyRead, this, &EngineUpdater::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &EngineUpdater::onReplyFinished);
}

void EngineUpdater::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int pct = static_cast<int>((bytesReceived * 100) / bytesTotal);
        emit statusEmitted(QString("🔄  Скачивание нового движка... %1%").arg(pct));
        emit progressUpdated(bytesReceived, bytesTotal);
    }
}

void EngineUpdater::onReadyRead() {
    if (m_reply && m_file) {
        m_file->write(m_reply->readAll());
    }
}

void EngineUpdater::onReplyFinished() {
    m_isUpdating = false;

    if (!m_reply || !m_file) {
        emit finished(false, "Неизвестная ошибка загрузки.");
        return;
    }

    if (m_reply->bytesAvailable() > 0) {
        m_file->write(m_reply->readAll());
    }
    m_file->close();

    if (m_reply->error() != QNetworkReply::NoError) {
        QString errStr = m_reply->errorString();
        m_file->remove();
        delete m_file;
        m_file = nullptr;
        m_reply->deleteLater();
        m_reply = nullptr;
        emit finished(false, QString("❌  Не удалось обновить движок — %1").arg(errStr));
        return;
    }

    m_reply->deleteLater();
    m_reply = nullptr;
    delete m_file;
    m_file = nullptr;

    // Replace existing yt-dlp.exe atomically
    if (QFile::exists(m_targetFilePath)) {
        QFile::remove(m_targetFilePath);
    }
    if (QFile::rename(m_tempFilePath, m_targetFilePath)) {
        emit logEmitted("✅  Движок (yt-dlp) успешно обновлен!");
        emit finished(true, "✅  Движок обновлён");
    } else {
        QFile::remove(m_tempFilePath);
        emit finished(false, "❌  Ошибка при замене файла yt-dlp.exe");
    }
}
