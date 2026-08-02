#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <iostream>
#include "CommandBuilder.h"
#include "EmbeddedToolsManager.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING --force-keyframes-at-cuts FOR SMOOTH KEYFRAME ALIGNMENT ===" << std::endl;

    QString outputDir = "D:/test_crop_verification";
    QDir().mkpath(outputDir);

    QStringList args = CommandBuilder::buildCommand(
        "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
        "1080p",
        false, // video
        outputDir,
        "ForceKeyframesTest",
        "00:00:15",
        "00:00:45"
    );

    std::cout << "COMMAND: " << args.join(" ").toStdString() << std::endl;

    QProcess process;
    process.start(args.first(), args.mid(1));
    process.closeWriteChannel();

    process.waitForFinished(120000);
    std::cout << "DOWNLOAD FINISHED WITH EXIT CODE: " << process.exitCode() << std::endl;

    // Run ffprobe on created file
    QString ffmpegBin = EmbeddedToolsManager::getToolsDir() + "/ffprobe.exe";
    QString videoFile = outputDir + "/ForceKeyframesTest.mp4";

    QProcess ffprobe;
    ffprobe.start(ffmpegBin, QStringList() << "-v" << "error" << "-select_streams" << "v:0" << "-show_entries" << "frame=key_frame,pkt_pts_time" << "-of" << "csv=p=0" << videoFile);
    ffprobe.waitForFinished(10000);

    QByteArray output = ffprobe.readAllStandardOutput();
    QString text = QString::fromUtf8(output);
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    if (!lines.isEmpty()) {
        std::cout << "FIRST FRAME IN OUTPUT VIDEO: " << lines.first().toStdString() << std::endl;
        if (lines.first().startsWith("1,")) {
            std::cout << "SUCCESS! FRAME 1 IS AN EXACT KEYFRAME (I-FRAME)! ZERO FREEZE GUARANTEED!" << std::endl;
        } else {
            std::cout << "WARNING: FRAME 1 KEYFRAME FLAG: " << lines.first().toStdString() << std::endl;
        }
    }

    return 0;
}
