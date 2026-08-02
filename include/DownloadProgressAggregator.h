#ifndef DOWNLOADPROGRESSAGGREGATOR_H
#define DOWNLOADPROGRESSAGGREGATOR_H

#include <QString>
#include <QVector>
#include "ProgressParser.h"

struct StreamInfo {
    double totalBytes = 0.0;
    double downloadedBytes = 0.0;
    bool isFinished = false;
};

class DownloadProgressAggregator {
public:
    DownloadProgressAggregator();

    void reset();
    void setAudioOnly(bool isAudioOnly) { m_isAudioOnly = isAudioOnly; }
    bool isAudioOnly() const { return m_isAudioOnly; }
    void setCropRange(double startSecs, double endSecs);

    ParsedProgress processLine(const QString &line);

    static double parseBytes(const QString &sizeStr);
    static QString formatBytes(double bytes);

private:
    int m_currentStreamIndex;
    QVector<StreamInfo> m_streams;
    double m_cropStartSeconds;
    double m_cropEndSeconds;
    double m_cropDurationSeconds;
    bool m_isAudioOnly;
};

#endif // DOWNLOADPROGRESSAGGREGATOR_H
