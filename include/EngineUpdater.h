#ifndef ENGINEUPDATER_H
#define ENGINEUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class EngineUpdater : public QObject {
    Q_OBJECT

public:
    explicit EngineUpdater(QObject *parent = nullptr);
    ~EngineUpdater();

    bool isUpdating() const { return m_isUpdating; }
    void startUpdate();

signals:
    void progressUpdated(qint64 bytesReceived, qint64 bytesTotal);
    void statusEmitted(const QString &status);
    void logEmitted(const QString &message);
    void finished(bool success, const QString &message);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onReplyFinished();
    void onReadyRead();

private:
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply;
    QFile *m_file;
    QString m_tempFilePath;
    QString m_targetFilePath;
    bool m_isUpdating;
};

#endif // ENGINEUPDATER_H
