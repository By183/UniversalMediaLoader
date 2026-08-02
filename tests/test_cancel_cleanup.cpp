#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    bool isAudioTest = (argc > 1 && QString(argv[1]) == "--audio");
    std::cout << "=== TESTING CANCEL CLEANUP FOR " << (isAudioTest ? "MP3 AUDIO CONVERSION" : "VIDEO MERGING") << " ===" << std::endl;

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";
    QDir().mkpath(outputDir);
    QString customName = isAudioTest ? "TestCancel_Audio" : "TestCancel_Video";

    QStringList args = CommandBuilder::buildCommand(url, "1080p", isAudioTest, outputDir, customName);

    ProcessRunner runner;
    bool cancelTriggered = false;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [&runner, &cancelTriggered, isAudioTest](const QString &line) {
        std::cout << "[LOG] " << line.toStdString() << "\n";

        bool triggerCondition = isAudioTest ? (line.contains("ExtractAudio") || line.contains("[ffmpeg]"))
                                            : (line.contains("[Merger]") || line.contains("Merging"));

        if (triggerCondition && !cancelTriggered) {
            cancelTriggered = true;
            std::cout << "\n>>> TRIGGER CONDITION DETECTED! INVOKING CANCEL() MID-PHASE <<<\n" << std::endl;
            QTimer::singleShot(200, [&runner]() {
                runner.cancel();
            });
        }
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app, outputDir, customName](bool success, int exitCode) {
        std::cout << "\nFINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << std::endl;

        QDir dir(outputDir);
        QStringList targetFiles = dir.entryList(QStringList() << (customName + ".*"), QDir::Files);

        std::cout << "\nOUTPUT DIRECTORY CLEANUP VERIFICATION:" << std::endl;
        std::cout << "  • Target Pattern Matching Files Count: " << targetFiles.size() << std::endl;
        for (const QString &f : targetFiles) {
            std::cout << "    - Found file left behind: " << f.toStdString() << std::endl;
        }

        bool isClean = targetFiles.isEmpty();
        std::cout << "CLEANUP VERIFICATION RESULT: " << (isClean ? "PASSED (Zero Partial Files Left)" : "FAILED (Partial File Left Behind!)") << std::endl;

        app.quit();
    });

    runner.startDownload(args, outputDir);

    return app.exec();
}
