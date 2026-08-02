#ifndef COMMANDBUILDER_H
#define COMMANDBUILDER_H

#include <QString>
#include <QStringList>

enum class DownloadMode {
    Video,
    Audio,
    Thumbnail
};

class CommandBuilder {
public:
    static QString sanitizeFilename(const QString &input);
    static QString formatTimestamp(const QString &input);

    static QString findYtDlpCmd();
    static QString findFfmpegDir();

    static QStringList buildCommand(
        const QString &url,
        const QString &quality,
        DownloadMode mode,
        const QString &outputDir,
        const QString &customName = QString(),
        const QString &timeStart = QString(),
        const QString &timeEnd = QString()
    );
};

#endif // COMMANDBUILDER_H
