#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <iostream>
#include "CommandBuilder.h"
#include "ProgressParser.h"
#include "DownloadProgressAggregator.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING CR ('\\r') SPLITTING FOR FFMPEG LIVE CROP PROGRESS ===" << std::endl;

    QString outputDir = "D:/test_crop_raw";
    QDir().mkpath(outputDir);

    QStringList args = CommandBuilder::buildCommand(
        "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
        "1080p",
        true, // audioOnly
        outputDir,
        "CrTestAudioCrop",
        "00:00:15",
        "00:01:45" // 90 seconds crop
    );

    std::cout << "COMMAND: " << args.join(" ").toStdString() << std::endl;

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    process.setProcessEnvironment(env);

    DownloadProgressAggregator aggregator;
    QString stdoutBuf;
    QString stderrBuf;

    auto processBuffer = [&aggregator](QString &buffer) {
        int idx;
        while ((idx = buffer.indexOf('\n')) != -1 || (idx = buffer.indexOf('\r')) != -1) {
            QString line = buffer.left(idx).trimmed();
            buffer.remove(0, idx + 1);

            if (line.isEmpty()) continue;

            ParsedProgress prog = aggregator.processLine(line);
            if (prog.type != ProgressType::IgnoredLine) {
                std::cout << "  [LIVE UI STATUS] " << prog.statusMessage.toStdString();
                if (prog.hasPercentage) {
                    std::cout << "  --> PROGRESS BAR: " << prog.percentage << "%";
                }
                std::cout << std::endl;
            }
        }
    };

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process, &stdoutBuf, &processBuffer]() {
        QByteArray data = process.readAllStandardOutput();
        stdoutBuf += QString::fromLocal8Bit(data);
        processBuffer(stdoutBuf);
    });

    QObject::connect(&process, &QProcess::readyReadStandardError, [&process, &stderrBuf, &processBuffer]() {
        QByteArray data = process.readAllStandardError();
        stderrBuf += QString::fromLocal8Bit(data);
        processBuffer(stderrBuf);
    });

    process.start(args.first(), args.mid(1));
    process.closeWriteChannel();

    process.waitForFinished(120000);
    std::cout << "FINISHED WITH EXIT CODE: " << process.exitCode() << std::endl;

    return 0;
}
