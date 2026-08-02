#include "CommandBuilder.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include "EmbeddedToolsManager.h"

QString CommandBuilder::sanitizeFilename(const QString &input) {
    if (input.trimmed().isEmpty()) return QString();
    QString cleaned = input;
    static QRegularExpression invalidChars(R"([\\/:*?"<>|])");
    cleaned.replace(invalidChars, "_");
    cleaned = cleaned.trimmed();
    if (cleaned.length() > 150) {
        cleaned = cleaned.left(150).trimmed();
    }
    return cleaned;
}

QString CommandBuilder::formatTimestamp(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return QString();

    if (trimmed.contains(':')) {
        return trimmed;
    }

    bool ok = false;
    int minutes = trimmed.toInt(&ok);
    if (ok && minutes >= 0) {
        int hours = minutes / 60;
        int mins = minutes % 60;
        return QString::asprintf("%02d:%02d:00", hours, mins);
    }

    return trimmed;
}

QString CommandBuilder::findYtDlpCmd() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString localBinary = QDir(appDir).filePath("yt-dlp.exe");
    if (QFileInfo::exists(localBinary)) {
        return QDir::toNativeSeparators(localBinary);
    }
    QString systemPath = QStandardPaths::findExecutable("yt-dlp");
    if (!systemPath.isEmpty()) {
        return QDir::toNativeSeparators(systemPath);
    }
    return "yt-dlp";
}

QString CommandBuilder::findFfmpegDir() {
    // 1. Check extracted private AppData location (%APPDATA%\UniversalMediaLoader\bin)
    QString appDataDir = EmbeddedToolsManager::getToolsDir();
    if (QFileInfo::exists(appDataDir + "/ffmpeg.exe") && QFileInfo::exists(appDataDir + "/ffprobe.exe")) {
        return QDir::toNativeSeparators(appDataDir);
    }
    // 2. Check local application directory ffmpeg/ folder
    QString appDir = QCoreApplication::applicationDirPath();
    QString localFfmpeg = QDir(appDir).filePath("ffmpeg/ffmpeg.exe");
    if (QFileInfo::exists(localFfmpeg)) {
        return QDir::toNativeSeparators(QDir(appDir).filePath("ffmpeg"));
    }
    // 3. Fallback to system PATH
    QString systemPath = QStandardPaths::findExecutable("ffmpeg");
    if (!systemPath.isEmpty()) {
        return QDir::toNativeSeparators(QFileInfo(systemPath).absolutePath());
    }
    return QString();
}

QStringList CommandBuilder::buildCommand(
    const QString &url,
    const QString &quality,
    DownloadMode mode,
    const QString &outputDir,
    const QString &customName,
    const QString &timeStart,
    const QString &timeEnd
) {
    QStringList args;

    args << findYtDlpCmd();

    args << "--newline";
    args << "--progress";
    args << "--no-cache-dir";
    args << "--user-agent" << "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36";
    args << "--add-header" << "Accept-Language: en-US,en;q=0.9";
    args << "--add-header" << "Sec-Fetch-Mode: navigate";

    bool isTikTok = url.contains("tiktok", Qt::CaseInsensitive);

    if (mode == DownloadMode::Thumbnail) {
        args << "--write-thumbnail";
        args << "--skip-download";
        args << "--convert-thumbnails" << "jpg";
    } else {
        // Disable thumbnail downloading and embedding for TikTok audio downloads to prevent extra standalone image files
        if (isTikTok && mode == DownloadMode::Audio) {
            args << "--no-write-thumbnail";
            args << "--no-embed-thumbnail";
        } else {
            args << "--convert-thumbnails" << "jpg";
            args << "--embed-thumbnail";
        }
    }

    QString ffmpegDir = findFfmpegDir();
    if (!ffmpegDir.isEmpty()) {
        args << "--ffmpeg-location" << ffmpegDir;
    }

    if (mode != DownloadMode::Thumbnail) {
        // Time-cropping section handling
        QString formattedStart = formatTimestamp(timeStart);
        QString formattedEnd = formatTimestamp(timeEnd);

        if (!formattedStart.isEmpty() || !formattedEnd.isEmpty()) {
            QString startSpec = formattedStart.isEmpty() ? "00:00:00" : formattedStart;
            QString endSpec = formattedEnd.isEmpty() ? "inf" : formattedEnd;
            args << "--download-sections" << QString("*%1-%2").arg(startSpec, endSpec);
            args << "--force-keyframes-at-cuts";
            args << "--postprocessor-args" << "ffmpeg:-avoid_negative_ts make_zero -fflags +genpts -async 1 -fps_mode cfr";
        }

        if (mode == DownloadMode::Audio) {
            args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << "0";
        } else {
            if (quality == "1080p") {
                args << "-f" << "bestvideo[height<=1080]+bestaudio/best[height<=1080]/best";
            } else if (quality == "720p") {
                args << "-f" << "bestvideo[height<=720]+bestaudio/best[height<=720]/best";
            } else if (quality == "480p") {
                args << "-f" << "bestvideo[height<=480]+bestaudio/best[height<=480]/best";
            } else if (quality == "360p") {
                args << "-f" << "bestvideo[height<=360]+bestaudio/best[height<=360]/best";
            } else {
                args << "-f" << "bestvideo+bestaudio/best";
            }
            args << "-S" << "fps,br";
            args << "--merge-output-format" << "mp4";
        }
    }

    QString sanitized = sanitizeFilename(customName);
    QString outputTemplate;

    if (!sanitized.isEmpty()) {
        if (mode == DownloadMode::Thumbnail) {
            outputTemplate = QDir(outputDir).filePath(sanitized + "_preview.%(ext)s");
        } else {
            outputTemplate = QDir(outputDir).filePath(sanitized + ".%(ext)s");
        }
    } else {
        if (mode == DownloadMode::Thumbnail) {
            outputTemplate = QDir(outputDir).filePath("%(title)s_preview.%(ext)s");
        } else if (mode == DownloadMode::Audio) {
            outputTemplate = QDir(outputDir).filePath("%(title)s.%(ext)s");
        } else {
            outputTemplate = QDir(outputDir).filePath("%(title)s [%(height)sp].%(ext)s");
        }
    }

    outputTemplate = QDir::toNativeSeparators(outputTemplate);
    args << "-o" << outputTemplate;

    // Clean TikTok tracking URL parameters (e.g. ?is_from_webapp=1&sender_device=pc)
    QString targetUrl = url.trimmed();
    if (isTikTok) {
        int qIdx = targetUrl.indexOf('?');
        if (qIdx != -1) {
            targetUrl = targetUrl.left(qIdx);
        }
    }

    args << targetUrl;

    return args;
}
