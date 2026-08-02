#ifndef PROGRESSPARSER_H
#define PROGRESSPARSER_H

#include <QString>

enum class ProgressType {
    ProgressWithEta,
    ProgressNoEta,
    PercentOnly,
    FragmentProgress,
    TotalFragments,
    Destination,
    Merger,
    AlreadyDownloaded,
    AudioExtraction,
    ExtractingInfo,
    GenericInfo,
    Error,
    Warning,
    IgnoredLine
};

struct ParsedProgress {
    ProgressType type = ProgressType::GenericInfo;
    bool hasPercentage = false;
    double percentage = 0.0;
    QString sizeStr;
    QString speedStr;
    QString etaStr;
    QString statusMessage;
    QString destinationFile;
    bool isError = false;
    bool isIndeterminate = false;
};

class ProgressParser {
public:
    static ParsedProgress parseLine(const QString &line);
};

#endif // PROGRESSPARSER_H
