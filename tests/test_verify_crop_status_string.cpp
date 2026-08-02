#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== VERIFYING CROP STATUS STRING FORMAT & PROGRESS BAR ===" << std::endl;

    QString outputDir = "D:/test_crop_verification";
    QDir().mkpath(outputDir);

    // 1. TEST MP3 CROP (15s to 105s -> 90s duration)
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 1: AUDIO (MP3) CROP STATUS STRING FORMAT" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        ProcessRunner runner;
        int statusCount = 0;
        bool foundFullFormat = false;

        QObject::connect(&runner, &ProcessRunner::progressUpdated, [&statusCount, &foundFullFormat](const ParsedProgress &prog) {
            if (prog.statusMessage.contains("Вырезание фрагмента")) {
                statusCount++;
                std::cout << "  [LIVE STATUS #" << statusCount << "] " << prog.statusMessage.toStdString();
                std::cout << " (hasPercentage=" << (prog.hasPercentage ? "TRUE" : "FALSE")
                          << ", pct=" << prog.percentage << "%)" << std::endl;

                if (prog.statusMessage.contains("%") && prog.statusMessage.contains("/") && prog.statusMessage.contains("Скорость")) {
                    foundFullFormat = true;
                }
            }
        });

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            true, // audioOnly
            outputDir,
            "VerifyFormatAudio",
            "00:00:15",
            "00:01:45"
        );

        runner.startDownload(args, outputDir);

        while (runner.isRunning()) {
            QCoreApplication::processEvents();
            QThread::msleep(50);
        }

        std::cout << "TEST 1 COMPLETED. FULL FORMAT VISIBLE: " << (foundFullFormat ? "YES (PASSED)" : "NO (FAILED)") << std::endl;
    }

    // 2. TEST VIDEO CROP (15s to 105s -> 90s duration)
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 2: VIDEO (MP4) CROP STATUS STRING FORMAT" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        ProcessRunner runner;
        int statusCount = 0;
        bool foundFullFormat = false;

        QObject::connect(&runner, &ProcessRunner::progressUpdated, [&statusCount, &foundFullFormat](const ParsedProgress &prog) {
            if (prog.statusMessage.contains("Вырезание фрагмента")) {
                statusCount++;
                std::cout << "  [LIVE STATUS #" << statusCount << "] " << prog.statusMessage.toStdString();
                std::cout << " (hasPercentage=" << (prog.hasPercentage ? "TRUE" : "FALSE")
                          << ", pct=" << prog.percentage << "%)" << std::endl;

                if (prog.statusMessage.contains("%") && prog.statusMessage.contains("/") && prog.statusMessage.contains("Скорость")) {
                    foundFullFormat = true;
                }
            }
        });

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            false, // video
            outputDir,
            "VerifyFormatVideo",
            "00:00:15",
            "00:01:45"
        );

        runner.startDownload(args, outputDir);

        while (runner.isRunning()) {
            QCoreApplication::processEvents();
            QThread::msleep(50);
        }

        std::cout << "TEST 2 COMPLETED. FULL FORMAT VISIBLE: " << (foundFullFormat ? "YES (PASSED)" : "NO (FAILED)") << std::endl;
    }

    return 0;
}
