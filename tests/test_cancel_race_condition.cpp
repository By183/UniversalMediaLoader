#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QThread>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

void checkDirectory(const QString &outputDir, const QString &stepName) {
    QDir dir(outputDir);
    QStringList files = dir.entryList(QDir::Files);
    std::cout << "\n=== GET-CHILDITEM REPORT AFTER " << stepName.toStdString() << " ===" << std::endl;
    std::cout << "Remaining files in folder (" << outputDir.toStdString() << "): " << files.size() << std::endl;
    for (const QString &f : files) {
        std::cout << "  - " << f.toStdString() << std::endl;
    }
    if (files.isEmpty()) {
        std::cout << "STATUS: CLEAN (Zero Files Remaining)\n" << std::endl;
    } else {
        std::cout << "STATUS: UNCLEAN (Files Left Behind!)\n" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";

    // Clean output directory before test start
    QDir dir(outputDir);
    if (dir.exists()) {
        for (const QString &f : dir.entryList(QDir::Files)) {
            QFile::remove(outputDir + "/" + f);
        }
    } else {
        dir.mkpath(outputDir);
    }

    std::cout << "=========================================================" << std::endl;
    std::cout << "   TESTING CANCEL RACE CONDITION & PAUSE/CANCEL SEQUENCE " << std::endl;
    std::cout << "=========================================================\n" << std::endl;

    ProcessRunner *runner = new ProcessRunner(&app);

    // STEP A: Download -> Pause -> Cancel
    std::cout << ">>> STEP A: START DOWNLOAD -> PAUSE -> CANCEL <<<" << std::endl;
    QStringList argsA = CommandBuilder::buildCommand(url, "720p", false, outputDir, "StepA_PauseCancel");
    runner->startDownload(argsA, outputDir);

    QTimer::singleShot(1200, [runner, &app, outputDir, url]() {
        std::cout << "  [Step A] Pausing download..." << std::endl;
        runner->pause();

        QTimer::singleShot(800, [runner, &app, outputDir, url]() {
            std::cout << "  [Step A] Cancelling download while paused..." << std::endl;
            runner->cancel();

            QThread::msleep(300);
            checkDirectory(outputDir, "STEP A (Pause -> Cancel)");

            // STEP B: Download -> Immediate Cancel (no pause)
            std::cout << ">>> STEP B: START DOWNLOAD -> IMMEDIATE CANCEL (NO PAUSE) <<<" << std::endl;
            QStringList argsB = CommandBuilder::buildCommand(url, "720p", false, outputDir, "StepB_ImmediateCancel1");
            runner->startDownload(argsB, outputDir);

            QTimer::singleShot(1500, [runner, &app, outputDir, url]() {
                std::cout << "  [Step B] Cancelling download..." << std::endl;
                runner->cancel();

                QThread::msleep(300);
                checkDirectory(outputDir, "STEP B (Immediate Cancel 1)");

                // STEP C: Download -> Immediate Cancel again
                std::cout << ">>> STEP C: START DOWNLOAD -> IMMEDIATE CANCEL AGAIN <<<" << std::endl;
                QStringList argsC = CommandBuilder::buildCommand(url, "720p", false, outputDir, "StepC_ImmediateCancel2");
                runner->startDownload(argsC, outputDir);

                QTimer::singleShot(1500, [runner, &app, outputDir]() {
                    std::cout << "  [Step C] Cancelling download..." << std::endl;
                    runner->cancel();

                    QThread::msleep(300);
                    checkDirectory(outputDir, "STEP C (Immediate Cancel 2)");

                    std::cout << "=========================================================" << std::endl;
                    std::cout << "   ALL THREE CANCEL STEPS COMPLETED SUCCESSFULY " << std::endl;
                    std::cout << "=========================================================" << std::endl;
                    app.quit();
                });
            });
        });
    });

    return app.exec();
}
