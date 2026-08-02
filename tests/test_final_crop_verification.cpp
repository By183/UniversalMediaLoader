#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== RUNNING FINAL VERIFICATION FOR AUDIO+CROP AND VIDEO+CROP ===" << std::endl;

    QString outputDir = "D:/test_crop_verification";
    QDir().mkpath(outputDir);

    // 1. TEST MP3 + CROP ("15" to "45" -> 30 mins)
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 1: MP3 DOWNLOAD WITH TIME CROP (15:00 - 45:00)" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        ProcessRunner runner;
        int updateCount = 0;

        QObject::connect(&runner, &ProcessRunner::progressUpdated, [&updateCount](const ParsedProgress &prog) {
            updateCount++;
            if (prog.hasPercentage || !prog.statusMessage.isEmpty()) {
                std::cout << "  [AUDIO PROGRESS #" << updateCount << "] " << prog.statusMessage.toStdString();
                if (prog.hasPercentage) {
                    std::cout << "  --> " << prog.percentage << "%";
                }
                std::cout << std::endl;
            }
        });

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            true, // audioOnly
            outputDir,
            "VerifiedAudioCrop",
            "00:00:15",
            "00:01:45"
        );

        QDateTime startTime = QDateTime::currentDateTime();
        runner.startDownload(args, outputDir);

        while (runner.isRunning()) {
            QCoreApplication::processEvents();
            QThread::msleep(50);
        }

        int elapsedSecs = startTime.secsTo(QDateTime::currentDateTime());
        std::cout << "TEST 1 COMPLETED IN " << elapsedSecs << " SECONDS. TOTAL PROGRESS UPDATES: " << updateCount << std::endl;
    }

    // 2. TEST VIDEO + CROP ("15" to "45" -> 30 mins)
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 2: VIDEO DOWNLOAD WITH TIME CROP (15:00 - 45:00)" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        ProcessRunner runner;
        int updateCount = 0;

        QObject::connect(&runner, &ProcessRunner::progressUpdated, [&updateCount](const ParsedProgress &prog) {
            updateCount++;
            if (prog.hasPercentage || !prog.statusMessage.isEmpty()) {
                std::cout << "  [VIDEO PROGRESS #" << updateCount << "] " << prog.statusMessage.toStdString();
                if (prog.hasPercentage) {
                    std::cout << "  --> " << prog.percentage << "%";
                }
                std::cout << std::endl;
            }
        });

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            false, // video
            outputDir,
            "VerifiedVideoCrop",
            "00:00:15",
            "00:01:45"
        );

        QDateTime startTime = QDateTime::currentDateTime();
        runner.startDownload(args, outputDir);

        while (runner.isRunning()) {
            QCoreApplication::processEvents();
            QThread::msleep(50);
        }

        int elapsedSecs = startTime.secsTo(QDateTime::currentDateTime());
        std::cout << "TEST 2 COMPLETED IN " << elapsedSecs << " SECONDS. TOTAL PROGRESS UPDATES: " << updateCount << std::endl;
    }

    return 0;
}
