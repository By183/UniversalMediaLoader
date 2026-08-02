#include "ProgressParser.h"
#include <QRegularExpression>
#include <QFileInfo>
#include <QtMath>

ParsedProgress ProgressParser::parseLine(const QString &line) {
    ParsedProgress result;
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return result;
    }

    // 0. Check for explicit Error or Warning lines
    if (trimmed.startsWith("ERROR:", Qt::CaseInsensitive) || trimmed.contains(" ERROR: ")) {
        result.type = ProgressType::Error;
        result.isError = true;
        if (trimmed.contains("is not a valid URL", Qt::CaseInsensitive) ||
            trimmed.contains("Unsupported URL", Qt::CaseInsensitive))
        {
            result.statusMessage = "❌  Не удалось распознать ссылку — убедитесь, что вставили правильный адрес";
        }
        else if (trimmed.contains("Unable to extract", Qt::CaseInsensitive) ||
                 trimmed.contains("signature", Qt::CaseInsensitive) ||
                 trimmed.contains("nsig", Qt::CaseInsensitive) ||
                 trimmed.contains("update yt-dlp", Qt::CaseInsensitive) ||
                 trimmed.contains("JS player", Qt::CaseInsensitive))
        {
            result.statusMessage = "🔄  Ошибка извлечения (возможно, изменились алгоритмы видеохостинга). Нажмите кнопку «🔄 Обновить движок»!";
        }
        else if (trimmed.contains("Video unavailable", Qt::CaseInsensitive) ||
                 trimmed.contains("This video is private", Qt::CaseInsensitive) ||
                 trimmed.contains("channel's members", Qt::CaseInsensitive) ||
                 trimmed.contains("subscriber-only", Qt::CaseInsensitive) ||
                 trimmed.contains("subscribers only", Qt::CaseInsensitive) ||
                 trimmed.contains("Sign in to confirm", Qt::CaseInsensitive) ||
                 trimmed.contains("does not exist", Qt::CaseInsensitive) ||
                 trimmed.contains("410", Qt::CaseInsensitive) ||
                 trimmed.contains("Gone", Qt::CaseInsensitive) ||
                 trimmed.contains("404", Qt::CaseInsensitive) ||
                 trimmed.contains("Not Found", Qt::CaseInsensitive))
        {
            result.statusMessage = "🔒  Контент недоступен (видео было удалено с сервера, приватное или доступ только по подписке)";
        }
        else {
            result.statusMessage = QString("❌  %1").arg(trimmed);
        }
        return result;
    }
    if (trimmed.startsWith("WARNING:", Qt::CaseInsensitive) || trimmed.contains(" WARNING: ")) {
        // Filter out non-actionable internal yt-dlp version & JS runtime warnings
        if (trimmed.contains("is older than 90 days", Qt::CaseInsensitive) ||
            trimmed.contains("No supported JavaScript runtime could be found", Qt::CaseInsensitive))
        {
            result.type = ProgressType::IgnoredLine;
            return result;
        }

        result.type = ProgressType::Warning;
        result.statusMessage = QString("⚠  %1").arg(trimmed);
        return result;
    }

    // 0.5 FFmpeg raw progress or console prompts
    if (trimmed.contains("Press [q] to stop", Qt::CaseInsensitive)) {
        result.type = ProgressType::GenericInfo;
        result.isIndeterminate = true;
        result.statusMessage = "✂  Вырезание фрагмента видео (обработка ffmpeg)...";
        return result;
    }

    static QRegularExpression rxFfmpegTime(R"(time=\s*([0-9]+:[0-9]+:[0-9]+(?:\.[0-9]+)?|[0-9]+:[0-9]+(?:\.[0-9]+)?))", QRegularExpression::CaseInsensitiveOption);
    static QRegularExpression rxFfmpegSpeed(R"(speed=\s*([0-9.]+\s*x))", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch tMatch = rxFfmpegTime.match(trimmed);
    if (tMatch.hasMatch()) {
        result.type = ProgressType::ProgressNoEta;
        result.isIndeterminate = true;
        result.etaStr = tMatch.captured(1);
        QRegularExpressionMatch sMatch = rxFfmpegSpeed.match(trimmed);
        if (sMatch.hasMatch()) {
            result.speedStr = sMatch.captured(1).trimmed();
        }
        return result;
    }

    // 1. Playlist / Item progress: [download] Downloading video 3 of 20
    static QRegularExpression rxPlayItem(R"(\[download\]\s+Downloading\s+(?:video|item|file)\s+(\d+)\s+of\s+(\d+))", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = rxPlayItem.match(trimmed);
    if (match.hasMatch()) {
        int current = match.captured(1).toInt();
        int total = match.captured(2).toInt();
        result.type = ProgressType::FragmentProgress;
        result.statusMessage = QString("📋  Видео %1 из %2 в плейлисте").arg(current).arg(total);
        return result;
    }

    // 2. Regular progress with ETA: [download] 45.3% of 150.00MiB at 5.20MiB/s ETA 00:15
    static QRegularExpression rxEta(R"(\[download\]\s+(\d+\.?\d*)%\s+of\s+~?\s*(\S+)\s+at\s+(\S+)\s+ETA\s+(\S+))");
    match = rxEta.match(trimmed);
    if (match.hasMatch()) {
        result.type = ProgressType::ProgressWithEta;
        result.hasPercentage = true;
        result.percentage = match.captured(1).toDouble();
        result.sizeStr = match.captured(2);
        result.speedStr = match.captured(3);
        result.etaStr = match.captured(4);
        result.statusMessage = QString("⬇  %1%  (%2)   •   %3   •   Осталось: %4")
                                   .arg(result.percentage, 0, 'f', 1)
                                   .arg(result.sizeStr)
                                   .arg(result.speedStr)
                                   .arg(result.etaStr);
        return result;
    }

    // 3. Percent only progress: [download] 45.3% of 150.00MiB
    static QRegularExpression rxPct(R"(\[download\]\s+(\d+\.?\d*)%\s+of\s+~?\s*(\S+))");
    match = rxPct.match(trimmed);
    if (match.hasMatch()) {
        result.type = ProgressType::PercentOnly;
        result.hasPercentage = true;
        result.percentage = match.captured(1).toDouble();
        result.sizeStr = match.captured(2);
        result.statusMessage = QString("⬇  %1%  (%2)").arg(result.percentage, 0, 'f', 1).arg(result.sizeStr);
        return result;
    }

    // 4. Fragment progress: [download] Downloading fragment 12 of 45
    static QRegularExpression rxFrag(R"(\[download\]\s+Downloading\s+fragment\s+(\d+)\s+of\s+(\d+))");
    match = rxFrag.match(trimmed);
    if (match.hasMatch()) {
        int current = match.captured(1).toInt();
        int total = match.captured(2).toInt();
        result.type = ProgressType::FragmentProgress;
        result.hasPercentage = true;
        result.percentage = (double)current / total * 100.0;
        result.statusMessage = QString("🧩  Фрагмент %1 из %2").arg(current).arg(total);
        return result;
    }

    // 5. Total fragments: [download] Total fragments: 45
    static QRegularExpression rxTotalFrag(R"(\[download\]\s+Total\s+fragments:\s+(\d+))");
    match = rxTotalFrag.match(trimmed);
    if (match.hasMatch()) {
        result.type = ProgressType::TotalFragments;
        result.statusMessage = QString("🧩  Всего фрагментов: %1").arg(match.captured(1));
        return result;
    }

    // 6. Destination file
    static QRegularExpression rxDest(R"(\[download\]\s+Destination:\s+(.+))");
    match = rxDest.match(trimmed);
    if (match.hasMatch()) {
        result.type = ProgressType::Destination;
        result.destinationFile = match.captured(1).trimmed();
        result.statusMessage = QString("📁  Сохранение: %1").arg(QFileInfo(result.destinationFile).fileName());
        return result;
    }

    // 7. Merger / Postprocessing
    if (trimmed.contains("[Merger]", Qt::CaseInsensitive) || trimmed.contains("Merging formats", Qt::CaseInsensitive)) {
        result.type = ProgressType::Merger;
        result.isIndeterminate = true;
        result.statusMessage = "🎬  Объединение видео и аудио потоков...";
        return result;
    }

    if (trimmed.contains("[ExtractAudio]", Qt::CaseInsensitive)) {
        result.type = ProgressType::AudioExtraction;
        result.isIndeterminate = true;
        result.statusMessage = "🎵  Конвертация и обработка медиафайла...";
        return result;
    }

    if (trimmed.contains("has already been downloaded", Qt::CaseInsensitive)) {
        result.type = ProgressType::AlreadyDownloaded;
        result.hasPercentage = true;
        result.percentage = 100.0;
        result.statusMessage = "✅  Файл уже скачан";
        return result;
    }

    if (trimmed.contains("[info] Extracting URL:", Qt::CaseInsensitive) || trimmed.contains("Downloading webpage", Qt::CaseInsensitive)) {
        result.type = ProgressType::ExtractingInfo;
        result.isIndeterminate = true;
        result.statusMessage = "🔍  Анализ ссылки и формата...";
        return result;
    }

    // Fallback
    result.type = ProgressType::GenericInfo;
    return result;
}
