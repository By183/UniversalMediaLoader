#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING AUTOMATIC PLAYLIST & SINGLE VIDEO BEHAVIOR ===" << std::endl;

    // 1. Single video URL test
    QString singleUrl = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QStringList singleArgs = CommandBuilder::buildCommand(singleUrl, "360p", false, "d:/Universal Media Loader/video_downloader/downloads_test");

    bool hasNoPlaylistFlag = singleArgs.contains("--no-playlist");
    bool hasYesPlaylistFlag = singleArgs.contains("--yes-playlist");

    std::cout << "Single Video Command Flags:" << std::endl;
    std::cout << "  • Contains --no-playlist : " << (hasNoPlaylistFlag ? "YES" : "NO (Correct)") << std::endl;
    std::cout << "  • Contains --yes-playlist: " << (hasYesPlaylistFlag ? "YES" : "NO (Correct)") << std::endl;

    // 2. Short Playlist URL test
    QString playlistUrl = "https://www.youtube.com/playlist?list=PL7E1A11100523C6D3";
    QStringList playlistArgs = CommandBuilder::buildCommand(playlistUrl, "360p", false, "d:/Universal Media Loader/video_downloader/downloads_test");

    std::cout << "\nTesting Playlist Download..." << std::endl;
    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [](const QString &line) {
        if (line.contains("Downloading video") || line.contains("Downloading item") || line.contains("[youtube:tab]") || line.contains("Destination:")) {
            std::cout << "[PLAYLIST LOG] " << line.toStdString() << "\n";
        }
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app](bool success, int exitCode) {
        std::cout << "\nPLAYLIST FINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << "\n";
        app.quit();
    });

    runner.startDownload(playlistArgs, "d:/Universal Media Loader/video_downloader/downloads_test");

    return app.exec();
}
