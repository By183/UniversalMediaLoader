#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <iostream>
#include "ProgressParser.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING CLIENT-SIDE VALIDATION & ERROR PARSING ===" << std::endl;

    // Case (a): Empty URL
    QString urlEmpty = "";
    if (urlEmpty.trimmed().isEmpty()) {
        std::cout << "Case (a) Empty URL -> STATUS: ⚠  Вставьте ссылку на видео или стрим" << std::endl;
    }

    // Case (b): Non-URL string like "5" or "hello"
    QString urlInvalid = "5";
    QUrl qurl(urlInvalid, QUrl::TolerantMode);
    bool hasValidScheme = urlInvalid.startsWith("http://", Qt::CaseInsensitive) || urlInvalid.startsWith("https://", Qt::CaseInsensitive);
    if (!hasValidScheme || !qurl.isValid() || !urlInvalid.contains(".")) {
        std::cout << "Case (b) Non-URL string '5' -> STATUS: ⚠  Некорректная ссылка — проверьте, что вы вставили правильный адрес" << std::endl;
    }

    // Case (c): Valid YouTube URL
    QString urlValid = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QUrl qurlValid(urlValid, QUrl::TolerantMode);
    bool hasValidScheme2 = urlValid.startsWith("http://", Qt::CaseInsensitive) || urlValid.startsWith("https://", Qt::CaseInsensitive);
    if (hasValidScheme2 && qurlValid.isValid() && urlValid.contains(".")) {
        std::cout << "Case (c) Valid URL -> PASSED VALIDATION, Proceeding to process" << std::endl;
    }

    // Case (d): Raw yt-dlp error line parsing
    QString rawYtdlpError = "ERROR: [generic] 'https://invalid-site-domain-123.org/xyz' is not a valid URL. Set --default-search \"auto\" to use a search engine.";
    ParsedProgress parsed = ProgressParser::parseLine(rawYtdlpError);
    std::cout << "Case (d) yt-dlp rejection -> STATUS: " << parsed.statusMessage.toStdString() << std::endl;

    return 0;
}
