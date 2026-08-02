#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QEventLoop>
#include <iostream>
#include "CommandBuilder.h"
#include "EmbeddedToolsManager.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING MP3 AUDIO THUMBNAIL EMBEDDING & STANDALONE JPG CLEANUP ===" << std::endl;

    EmbeddedToolsManager::ensureToolsExtracted();

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "D:/Universal Media Loader1/video_downloader/downloads_mp3_test";
    QDir().mkpath(outputDir);

    // Clear test folder first
    QDir dir(outputDir);
    for (const QFileInfo &f : dir.entryInfoList(QDir::Files)) {
        QFile::remove(f.absoluteFilePath());
    }

    // 1. Generate Command & Verify --embed-thumbnail Presence
    QStringList args = CommandBuilder::buildCommand(url, "1080p", true, outputDir, "Mp3EmbedTest", "0", "0:05");
    std::cout << "Generated MP3 Command Args:" << std::endl;
    for (const QString &arg : args) {
        std::cout << "  " << arg.toStdString() << std::endl;
    }

    bool hasEmbedThumb = args.contains("--embed-thumbnail");
    std::cout << "\nCHECK 1: --embed-thumbnail present in args? " << (hasEmbedThumb ? "YES [PASS]" : "NO [FAIL]") << std::endl;

    // 2. Execute Download via ProcessRunner
    ProcessRunner runner;
    QEventLoop loop;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [](const QString &line) {
        std::cout << "[LOG] " << line.toStdString() << std::endl;
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&loop](bool success, int exitCode) {
        std::cout << "\nProcessRunner finished with success=" << success << " exitCode=" << exitCode << std::endl;
        loop.quit();
    });

    std::cout << "\nExecuting MP3 Download via ProcessRunner..." << std::endl;
    runner.startDownload(args, outputDir);
    loop.exec();

    // 3. Inspect Files in Output Folder
    std::cout << "\nFiles remaining in " << outputDir.toStdString() << ":" << std::endl;
    QFileInfoList allFiles = dir.entryInfoList(QDir::Files);
    bool hasJpgLeftover = false;
    bool hasMp3File = false;
    QString mp3Path;

    for (const QFileInfo &f : allFiles) {
        std::cout << "  - FILE: " << f.fileName().toStdString() << " (" << f.size() << " bytes)" << std::endl;
        if (f.suffix().toLower() == "jpg" || f.suffix().toLower() == "webp" || f.suffix().toLower() == "png") {
            hasJpgLeftover = true;
        }
        if (f.suffix().toLower() == "mp3") {
            hasMp3File = true;
            mp3Path = f.absoluteFilePath();
        }
    }

    std::cout << "\nCHECK 2: Standalone .jpg thumbnail cleaned up (none leftover)? "
              << (!hasJpgLeftover ? "YES [PASS]" : "NO [FAIL - Standalone JPG Leftover!]") << std::endl;
    std::cout << "CHECK 3: MP3 file created? " << (hasMp3File ? "YES [PASS]" : "NO [FAIL]") << std::endl;

    // 4. Verify Embedded Artwork inside MP3 via ffprobe
    if (hasMp3File) {
        QString ffprobeBin = EmbeddedToolsManager::getToolsDir() + "/ffprobe.exe";
        QProcess probeProc;
        probeProc.start(ffprobeBin, QStringList() << "-show_streams" << "-select_streams" << "v" << mp3Path);
        probeProc.waitForFinished(5000);
        QString probeOut = QString::fromUtf8(probeProc.readAllStandardOutput());

        bool hasAttachedPic = probeOut.contains("DISPOSITION:attached_pic=1") || probeOut.contains("codec_name=mjpeg");
        std::cout << "CHECK 4: ffprobe detected ID3v2 APIC attached cover art stream inside MP3? "
                  << (hasAttachedPic ? "YES [PASS]" : "NO [FAIL]") << std::endl;
    }

    return 0;
}
