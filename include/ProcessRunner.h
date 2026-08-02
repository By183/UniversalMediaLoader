#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QObject>
#include <QProcess>
#include <QVector>
#include <QDateTime>
#include <QSet>
#include "ProgressParser.h"
#include "DownloadProgressAggregator.h"

class ProcessRunner : public QObject {
    Q_OBJECT

public:
    explicit ProcessRunner(QObject *parent = nullptr);
    ~ProcessRunner();

    bool isRunning() const;
    bool isPaused() const { return m_isPaused; }
    qint64 processId() const;

    void startDownload(const QStringList &args, const QString &outputDir);
    bool pause();
    bool resume();
    void cancel();

signals:
    void progressUpdated(const ParsedProgress &progress);
    void logOutputEmitted(const QString &rawLine);
    void finished(bool success, int exitCode);
    void pauseStateChanged(bool isPaused);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void processLineBuffer(QString &buffer, bool isErrorStream);
    void cleanupTempFiles();
    void cleanupAudioCoverTempFiles();
    QVector<unsigned long> getProcessTreePids(unsigned long parentPid);
    void suspendProcessTree(unsigned long parentPid);
    void resumeProcessTree(unsigned long parentPid);
    void killProcessTree(unsigned long parentPid);

    QProcess *m_process;
    void *m_jobObject;
    QString m_outputDir;
    QString m_targetFilePath;
    QSet<QString> m_createdFiles;
    QDateTime m_startTime;
    QString m_stdoutBuffer;
    QString m_stderrBuffer;
    bool m_isPaused;
    bool m_isCancelled;
    bool m_isThumbnailMode;
    DownloadProgressAggregator m_aggregator;
};

#endif // PROCESSRUNNER_H
