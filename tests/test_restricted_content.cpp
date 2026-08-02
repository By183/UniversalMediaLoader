#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "ProgressParser.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING ACCESS-RESTRICTED CONTENT PATTERN MATCHING ===" << std::endl;

    // 1. Literal captured yt-dlp error lines
    QString lineYtPrivate = "ERROR: [youtube] m7Bc3jbAbp0: Video unavailable";
    QString lineTwitchNotExist = "ERROR: [twitch:vod] 1000000000: Video 1000000000 does not exist";
    QString lineMembersOnly = "ERROR: [youtube] abc123xyz: This video is available to this channel's members";
    QString lineSubscribersOnly = "ERROR: [twitch:vod] 987654321: This VOD is subscriber-only";

    ParsedProgress p1 = ProgressParser::parseLine(lineYtPrivate);
    ParsedProgress p2 = ProgressParser::parseLine(lineTwitchNotExist);
    ParsedProgress p3 = ProgressParser::parseLine(lineMembersOnly);
    ParsedProgress p4 = ProgressParser::parseLine(lineSubscribersOnly);

    std::cout << "Restricted Matches:" << std::endl;
    std::cout << "  • YT Private     : " << p1.statusMessage.toStdString() << std::endl;
    std::cout << "  • Twitch NotExist: " << p2.statusMessage.toStdString() << std::endl;
    std::cout << "  • YT Members     : " << p3.statusMessage.toStdString() << std::endl;
    std::cout << "  • Twitch Sub-Only: " << p4.statusMessage.toStdString() << std::endl;

    // 2. REGRESSION CHECK on normal working lines
    std::cout << "\n=== REGRESSION CHECK (PUBLIC WORKING LINES) ===" << std::endl;

    QString lineYtOk = "[download] 45.3% of 150.00MiB at 5.20MiB/s ETA 00:15";
    QString lineTwitchOk = "[download] Downloading fragment 5 of 120";
    QString lineMergerOk = "[Merger] Merging formats into \"video.mp4\"";
    QString lineGenericOk = "[youtube] aqz-KE-bpKQ: Downloading webpage";

    ParsedProgress r1 = ProgressParser::parseLine(lineYtOk);
    ParsedProgress r2 = ProgressParser::parseLine(lineTwitchOk);
    ParsedProgress r3 = ProgressParser::parseLine(lineMergerOk);
    ParsedProgress r4 = ProgressParser::parseLine(lineGenericOk);

    bool hasFalsePositive = (r1.statusMessage.contains("Контент недоступен") ||
                             r2.statusMessage.contains("Контент недоступен") ||
                             r3.statusMessage.contains("Контент недоступен") ||
                             r4.statusMessage.contains("Контент недоступен"));

    std::cout << "  • YT Download Line : " << r1.statusMessage.toStdString() << std::endl;
    std::cout << "  • Twitch Frag Line : " << r2.statusMessage.toStdString() << std::endl;
    std::cout << "  • Merger Line      : " << r3.statusMessage.toStdString() << std::endl;
    std::cout << "  • Info Line        : " << r4.statusMessage.toStdString() << std::endl;

    std::cout << "\nREGRESSION RESULT: " << (hasFalsePositive ? "FAILED (False Positive)" : "PASSED (Zero False Positives)") << std::endl;

    return 0;
}
