#include <iostream>
#include "CommandBuilder.h"
#include "ProgressParser.h"

int main() {
    std::cout << "=== TESTING COMMANDBUILDER ===" << std::endl;
    QStringList cmd1 = CommandBuilder::buildCommand("https://youtube.com/watch?v=123", "1080p", false, "D:/downloads", "Test: Video?*");
    std::cout << "Video Cmd: " << cmd1.join(" ").toStdString() << "\n\n";

    QStringList cmd2 = CommandBuilder::buildCommand("https://twitch.tv/stream", "720p", true, "D:/downloads", "");
    std::cout << "Audio Cmd: " << cmd2.join(" ").toStdString() << "\n\n";

    std::cout << "=== TESTING PROGRESSPARSER ===" << std::endl;
    QStringList testLines = {
        "[download]  45.3% of 150.00MiB at 5.20MiB/s ETA 00:15",
        "[download]  82.1% of ~300.00MiB at 12.40MiB/s",
        "[download] Downloading fragment 25 of 100",
        "[download] Destination: D:/downloads/MyVideo.mp4",
        "[Merger] Merging formats into \"D:/downloads/MyVideo.mp4\"",
        "ExtractAudio: Converting video to mp3",
        "[download] video.mp4 has already been downloaded"
    };

    for (const QString &line : testLines) {
        ParsedProgress res = ProgressParser::parseLine(line);
        std::cout << "INPUT:  " << line.toStdString() << "\n";
        std::cout << "STATUS: " << res.statusMessage.toStdString() << "\n";
        std::cout << "PCT:    " << res.percentage << "% | SPEED: " << res.speedStr.toStdString() << " | ETA: " << res.etaStr.toStdString() << "\n\n";
    }

    return 0;
}
