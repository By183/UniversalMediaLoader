#include "DownloadProgressAggregator.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QtMath>

static double parseTimeToSeconds(const QString &tsStr) {
    if (tsStr.trimmed().isEmpty()) return -1.0;
    QString str = tsStr.trimmed();

    if (str.compare("inf", Qt::CaseInsensitive) == 0 || str.compare("infinity", Qt::CaseInsensitive) == 0) {
        return -1.0;
    }

    if (!str.contains(':')) {
        bool ok = false;
        double mins = str.toDouble(&ok);
        if (ok && mins >= 0) {
            return mins * 60.0;
        }
        return -1.0;
    }

    QStringList parts = str.split(':');
    if (parts.size() == 3) {
        return parts[0].toDouble() * 3600.0 + parts[1].toDouble() * 60.0 + parts[2].toDouble();
    }
    if (parts.size() == 2) {
        return parts[0].toDouble() * 60.0 + parts[1].toDouble();
    }
    return -1.0;
}

static QString formatSeconds(double totalSecs) {
    if (totalSecs <= 0) return "00:00";
    int s = static_cast<int>(totalSecs);
    int h = s / 3600;
    int m = (s % 3600) / 60;
    int sec = s % 60;
    if (h > 0) {
        return QString::asprintf("%02d:%02d:%02d", h, m, sec);
    }
    return QString::asprintf("%02d:%02d", m, sec);
}

DownloadProgressAggregator::DownloadProgressAggregator() {
    reset();
}

void DownloadProgressAggregator::reset() {
    m_currentStreamIndex = -1;
    m_streams.clear();
    m_cropStartSeconds = -1.0;
    m_cropEndSeconds = -1.0;
    m_cropDurationSeconds = -1.0;
    m_isAudioOnly = false;
}

void DownloadProgressAggregator::setCropRange(double startSecs, double endSecs) {
    m_cropStartSeconds = startSecs;
    m_cropEndSeconds = endSecs;
    if (endSecs > startSecs && startSecs >= 0) {
        m_cropDurationSeconds = endSecs - startSecs;
    } else {
        m_cropDurationSeconds = -1.0;
    }
}

double DownloadProgressAggregator::parseBytes(const QString &sizeStr) {
    if (sizeStr.isEmpty()) return 0.0;
    QString str = sizeStr.trimmed();

    double factor = 1.0;
    if (str.endsWith("GiB", Qt::CaseInsensitive) || str.endsWith("GB", Qt::CaseInsensitive)) {
        factor = 1024.0 * 1024.0 * 1024.0;
    } else if (str.endsWith("MiB", Qt::CaseInsensitive) || str.endsWith("MB", Qt::CaseInsensitive)) {
        factor = 1024.0 * 1024.0;
    } else if (str.endsWith("KiB", Qt::CaseInsensitive) || str.endsWith("KB", Qt::CaseInsensitive)) {
        factor = 1024.0;
    } else if (str.endsWith("B", Qt::CaseInsensitive)) {
        factor = 1.0;
    }

    QString numPart;
    for (QChar c : str) {
        if (c.isDigit() || c == '.') {
            numPart.append(c);
        }
    }
    return numPart.toDouble() * factor;
}

QString DownloadProgressAggregator::formatBytes(double bytes) {
    if (bytes >= 1024.0 * 1024.0 * 1024.0) {
        return QString("%1 GiB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
    if (bytes >= 1024.0 * 1024.0) {
        return QString("%1 MiB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }
    if (bytes >= 1024.0) {
        return QString("%1 KiB").arg(bytes / 1024.0, 0, 'f', 1);
    }
    return QString("%1 B").arg(bytes, 0, 'f', 0);
}

ParsedProgress DownloadProgressAggregator::processLine(const QString &line) {
    ParsedProgress parsed = ProgressParser::parseLine(line);

    // 1. Track crop range from yt-dlp time ranges output: [info] ... Downloading 1 time ranges: 15.0-105.0 or 300.0-inf
    static QRegularExpression rxTimeRanges(R"(Downloading\s+\d+\s+time\s+ranges:\s*(\d+(?:\.\d+)?)-(\S+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch rangeMatch = rxTimeRanges.match(line);
    if (rangeMatch.hasMatch()) {
        double startSec = parseTimeToSeconds(rangeMatch.captured(1));
        double endSec = parseTimeToSeconds(rangeMatch.captured(2));
        setCropRange(startSec, endSec);
    }

    // 2. Track crop range from app launch log: ✂ Обрезка фрагмента: От 15 До конец / 45
    static QRegularExpression rxCropInfo(R"(Обрезка\s+фрагмента:\s+От\s+(\S+)\s+До\s+(\S+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch cropMatch = rxCropInfo.match(line);
    if (cropMatch.hasMatch()) {
        double startSec = parseTimeToSeconds(cropMatch.captured(1));
        double endSec = parseTimeToSeconds(cropMatch.captured(2));
        setCropRange(startSec, endSec);
    }

    // 3. Track crop range from --download-sections args: --download-sections *00:15:00-inf
    static QRegularExpression rxSectionInfo(R"(--download-sections\s+\*(\S+)-(\S+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch secMatch = rxSectionInfo.match(line);
    if (secMatch.hasMatch()) {
        double startSec = parseTimeToSeconds(secMatch.captured(1));
        double endSec = parseTimeToSeconds(secMatch.captured(2));
        setCropRange(startSec, endSec);
    }

    // Override generic prompt with audio/video specific prompt
    if (line.contains("Press [q] to stop", Qt::CaseInsensitive)) {
        parsed.type = ProgressType::GenericInfo;
        parsed.isIndeterminate = true;
        parsed.statusMessage = m_isAudioOnly
            ? "✂  Вырезание фрагмента аудио (обработка ffmpeg)..."
            : "✂  Вырезание фрагмента видео (обработка ffmpeg)...";
        return parsed;
    }

    // Independent fail-safe FFmpeg time and speed extraction
    static QRegularExpression rxTime(R"(time=\s*([0-9]+:[0-9]+:[0-9]+(?:\.[0-9]+)?|[0-9]+:[0-9]+(?:\.[0-9]+)?))", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression rxSpeed(R"(speed=\s*([0-9.]+\s*x))", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch tMatch = rxTime.match(line);
    if (tMatch.hasMatch()) {
        QString timeStr = tMatch.captured(1);
        double currentSecs = parseTimeToSeconds(timeStr);

        QString speedStr;
        QRegularExpressionMatch sMatch = rxSpeed.match(line);
        if (sMatch.hasMatch()) {
            speedStr = sMatch.captured(1).trimmed();
        }

        QString mediaTypeStr = m_isAudioOnly ? "аудио" : "видео";

        if (currentSecs >= 0 && m_cropDurationSeconds > 0) {
            double pct = (currentSecs / m_cropDurationSeconds) * 100.0;
            if (pct > 99.9) pct = 99.9;
            parsed.hasPercentage = true;
            parsed.percentage = pct;
            parsed.isIndeterminate = false;
            if (!speedStr.isEmpty()) {
                parsed.statusMessage = QString("✂  Вырезание фрагмента %1: %2% (%3 / %4)   •   Скорость: %5")
                                           .arg(mediaTypeStr)
                                           .arg(pct, 0, 'f', 1)
                                           .arg(formatSeconds(currentSecs))
                                           .arg(formatSeconds(m_cropDurationSeconds))
                                           .arg(speedStr);
            } else {
                parsed.statusMessage = QString("✂  Вырезание фрагмента %1: %2% (%3 / %4)")
                                           .arg(mediaTypeStr)
                                           .arg(pct, 0, 'f', 1)
                                           .arg(formatSeconds(currentSecs))
                                           .arg(formatSeconds(m_cropDurationSeconds));
            }
        } else if (currentSecs >= 0) {
            parsed.isIndeterminate = true;
            if (!speedStr.isEmpty()) {
                parsed.statusMessage = QString("✂  Вырезание фрагмента %1: %2   •   Скорость: %3")
                                           .arg(mediaTypeStr)
                                           .arg(formatSeconds(currentSecs))
                                           .arg(speedStr);
            } else {
                parsed.statusMessage = QString("✂  Вырезание фрагмента %1: %2")
                                           .arg(mediaTypeStr)
                                           .arg(formatSeconds(currentSecs));
            }
        }
        return parsed;
    }

    if (line.contains("[download] Destination:")) {
        m_currentStreamIndex++;
        if (m_currentStreamIndex >= m_streams.size()) {
            m_streams.resize(m_currentStreamIndex + 1);
        }
    }

    if (parsed.hasPercentage && m_currentStreamIndex >= 0 && m_currentStreamIndex < m_streams.size()) {
        double currentTotal = parseBytes(parsed.sizeStr);
        if (currentTotal > 0) {
            m_streams[m_currentStreamIndex].totalBytes = currentTotal;
        }
        if (m_streams[m_currentStreamIndex].totalBytes > 0) {
            m_streams[m_currentStreamIndex].downloadedBytes = (m_streams[m_currentStreamIndex].totalBytes * parsed.percentage) / 100.0;
        }
        if (parsed.percentage >= 99.9) {
            m_streams[m_currentStreamIndex].isFinished = true;
        }

        double totalDownloadedAll = 0.0;
        double totalSizeAll = 0.0;
        for (const StreamInfo &s : m_streams) {
            totalDownloadedAll += s.downloadedBytes;
            totalSizeAll += s.totalBytes;
        }

        if (totalSizeAll > 0) {
            double combinedPct = (totalDownloadedAll / totalSizeAll) * 100.0;
            if (combinedPct > 100.0) combinedPct = 100.0;
            parsed.percentage = combinedPct;

            QString dlStr = formatBytes(totalDownloadedAll);
            QString totalStr = formatBytes(totalSizeAll);

            if (!parsed.speedStr.isEmpty()) {
                parsed.statusMessage = QString("⬇  %1%  (%2 / %3)   •   %4")
                                           .arg(combinedPct, 0, 'f', 1)
                                           .arg(dlStr)
                                           .arg(totalStr)
                                           .arg(parsed.speedStr);
            } else {
                parsed.statusMessage = QString("⬇  %1%  (%2 / %3)")
                                           .arg(combinedPct, 0, 'f', 1)
                                           .arg(dlStr)
                                           .arg(totalStr);
            }
        }
    }

    return parsed;
}
