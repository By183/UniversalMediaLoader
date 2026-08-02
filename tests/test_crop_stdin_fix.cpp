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

    std::cout << "=== TESTING STDIN CLOSE FIX FOR CROP DOWNLOADS ===" << std::endl;

    QString outputDir = "D:/test_crop_raw";
    QDir().mkpath(outputDir);

    // 1. TEST AUDIO + CROP (MP3) with closeWriteChannel()
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 1: MP3 DOWNLOAD WITH CROP + closeWriteChannel()" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            true, // audioOnly
            outputDir,
            "TestAudioCropFixed",
            "00:00:30",
            "00:01:30"
        );

        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONIOENCODING", "utf-8");
        env.insert("PYTHONUTF8", "1");
        process.setProcessEnvironment(env);

        QString program = args.first();
        QStringList arguments = args.mid(1);

        QFile rawLogFile("D:/test_crop_raw/fixed_audio_crop.log");
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
        process.closeWriteChannel(); // <--- CRITICAL FIX: Close stdin channel immediately so ffmpeg never hangs waiting for stdin input!

        bool finished = process.waitForFinished(60000); // 60s timeout
        std::cout << "\nTEST 1 FINISHED: " << (finished ? "YES" : "NO (TIMEOUT)") << " EXIT CODE: " << process.exitCode() << std::endl;
        rawLogFile.close();
    }

    // 2. TEST VIDEO + CROP (MP4) with closeWriteChannel()
    {
        std::cout << "\n--------------------------------------------------" << std::endl;
        std::cout << "TEST 2: VIDEO DOWNLOAD WITH CROP + closeWriteChannel()" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;

        QStringList args = CommandBuilder::buildCommand(
            "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
            "1080p",
            false, // video
            outputDir,
            "TestVideoCropFixed",
            "00:00:30",
            "00:01:30"
        );

        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PYTHONIOENCODING", "utf-8");
        env.insert("PYTHONUTF8", "1");
        process.setProcessEnvironment(env);

        QString program = args.first();
        QStringList arguments = args.mid(1);

        QFile rawLogFile("D:/test_crop_raw/fixed_video_crop.log");
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
        process.closeWriteChannel(); // <--- CRITICAL FIX: Close stdin channel immediately so ffmpeg never hangs waiting for stdin input!

        bool finished = process.waitForFinished(60000); // 60s timeout
        std::cout << "\nTEST 2 FINISHED: " << (finished ? "YES" : "NO (TIMEOUT)") << " EXIT CODE: " << process.exitCode() << std::endl;
        rawLogFile.close();
    }

    return 0;
}
