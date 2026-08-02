#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING FIXES 1, 2, 3a, 3b (CYRILLIC PATH + FFMPEG SECTION DOWNLOAD) ===" << std::endl;

    // 1. Verify timestamp formatting
    QString startRaw = "50";
    QString endRaw = "52";
    QString formattedStart = CommandBuilder::formatTimestamp(startRaw);
    QString formattedEnd = CommandBuilder::formatTimestamp(endRaw);

    std::cout << "Timestamp Formatting Check:" << std::endl;
    std::cout << "  • Input '50' -> Formatted: " << formattedStart.toStdString() << " (Expected 00:50:00 - Minute 50)" << std::endl;
    std::cout << "  • Input '52' -> Formatted: " << formattedEnd.toStdString() << " (Expected 00:52:00 - Minute 52)" << std::endl;

    // 2. Build command for Cyrillic target directory
    QString url = "https://www.twitch.tv/videos/2834308857";
    QString outputDir = "d:/тест проги";
    QDir().mkpath(outputDir);

    QString customName = "TwitchCrop_Min50_52";

    QStringList args = CommandBuilder::buildCommand(url, "360p", false, outputDir, customName, startRaw, endRaw);

    std::cout << "\nCOMMAND GENERATED:" << std::endl;
    for (const QString &arg : args) {
        std::cout << arg.toStdString() << " ";
    }
    std::cout << "\n\nLaunching download..." << std::endl;

    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [](const QString &line) {
        if (line.contains("frame=") || line.contains("[download]") || line.contains("Destination:")) {
            std::cout << "[PROGRESS LOG] " << line.toStdString() << "\n";
        }
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app, outputDir, customName](bool success, int exitCode) {
        std::cout << "\nFINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << std::endl;

        QDir dir(outputDir);
        QString expectedFile = dir.filePath(customName + ".mp4");
        bool fileExists = QFile::exists(expectedFile);
        qint64 fileSize = QFile(expectedFile).size();

        std::cout << "\nCYRILLIC OUTPUT FOLDER VERIFICATION:" << std::endl;
        std::cout << "  • Path         : " << expectedFile.toStdString() << std::endl;
        std::cout << "  • File Exists  : " << (fileExists ? "YES" : "NO") << std::endl;
        std::cout << "  • File Size    : " << fileSize << " bytes" << std::endl;

        app.quit();
    });

    runner.startDownload(args, outputDir);

    return app.exec();
}
