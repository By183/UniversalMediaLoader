#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include "CommandBuilder.h"
#include "EmbeddedToolsManager.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== RUNNING FULL RAW LOG DIAGNOSTIC FOR AUDIO+CROP AND VIDEO+CROP ===" << std::endl;

    QString outputDir = "D:/test_crop_raw";
    QDir().mkpath(outputDir);

    // 1. TEST AUDIO + CROP (MP3) - 00:00:30 to 00:02:30
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 1: MP3 DOWNLOAD WITH CROP (00:00:30 - 00:02:30)" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            true, // audioOnly
            outputDir,
            "RawTestAudioCrop",
            "00:00:30",
            "00:02:30"
        );

        std::cout << "COMMAND: " << args.join(" ").toStdString() << std::endl;

        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONIOENCODING", "utf-8");
        env.insert("PYTHONUTF8", "1");
        process.setProcessEnvironment(env);

        QString program = args.first();
        QStringList arguments = args.mid(1);

        QFile rawLogFile("D:/test_crop_raw/raw_audio_crop.log");
        rawLogFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&rawLogFile);

        QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process, &outStream]() {
            QByteArray data = process.readAllStandardOutput();
            QString text = QString::fromLocal8Bit(data);
            std::cout << "[STDOUT] " << text.toStdString();
            outStream << "[STDOUT] " << text;
            outStream.flush();
        });

        QObject::connect(&process, &QProcess::readyReadStandardError, [&process, &outStream]() {
            QByteArray data = process.readAllStandardError();
            QString text = QString::fromLocal8Bit(data);
            std::cout << "[STDERR] " << text.toStdString();
            outStream << "[STDERR] " << text;
            outStream.flush();
        });

        process.start(program, arguments);
        bool finished = process.waitForFinished(180000); // 3 minutes timeout
        std::cout << "\nTEST 1 FINISHED: " << (finished ? "YES" : "NO (TIMEOUT)") << " EXIT CODE: " << process.exitCode() << std::endl;
        rawLogFile.close();
    }

    // 2. TEST VIDEO + CROP (1080p MP4) - 00:00:30 to 00:02:30
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 2: VIDEO DOWNLOAD WITH CROP (00:00:30 - 00:02:30)" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            false, // video
            outputDir,
            "RawTestVideoCrop",
            "00:00:30",
            "00:02:30"
        );

        std::cout << "COMMAND: " << args.join(" ").toStdString() << std::endl;

        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONIOENCODING", "utf-8");
        env.insert("PYTHONUTF8", "1");
        process.setProcessEnvironment(env);

        QString program = args.first();
        QStringList arguments = args.mid(1);

        QFile rawLogFile("D:/test_crop_raw/raw_video_crop.log");
        rawLogFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&rawLogFile);

        QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process, &outStream]() {
            QByteArray data = process.readAllStandardOutput();
            QString text = QString::fromLocal8Bit(data);
            std::cout << "[STDOUT] " << text.toStdString();
            outStream << "[STDOUT] " << text;
            outStream.flush();
        });

        QObject::connect(&process, &QProcess::readyReadStandardError, [&process, &outStream]() {
            QByteArray data = process.readAllStandardError();
            QString text = QString::fromLocal8Bit(data);
            std::cout << "[STDERR] " << text.toStdString();
            outStream << "[STDERR] " << text;
            outStream.flush();
        });

        process.start(program, arguments);
        bool finished = process.waitForFinished(180000); // 3 minutes timeout
        std::cout << "\nTEST 2 FINISHED: " << (finished ? "YES" : "NO (TIMEOUT)") << " EXIT CODE: " << process.exitCode() << std::endl;
        rawLogFile.close();
    }

    return 0;
}
