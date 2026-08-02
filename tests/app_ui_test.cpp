#include <QApplication>
#include <QTimer>
#include <QEventLoop>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>
#include "MainWindow.h"

// Real App UI Integration Test Runner
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    std::cout << "=========================================================" << std::endl;
    std::cout << "  STARTING REAL C++/Qt APP UI INTEGRATION TEST PASS     " << std::endl;
    std::cout << "=========================================================" << std::endl;

    QString testDirStr = "D:/test_ui_scenarios";
    QDir testDir(testDirStr);
    if (testDir.exists()) {
        testDir.removeRecursively();
    }
    QDir().mkpath(testDirStr);

    MainWindow window;
    window.show();

    // Configure save path on sidebar
    SidebarWidget *sidebar = window.findChild<SidebarWidget*>();
    MainPanel *mainPanel = window.findChild<MainPanel*>();

    if (!sidebar || !mainPanel) {
        std::cerr << "FAIL: Could not find SidebarWidget or MainPanel in MainWindow!" << std::endl;
        return 1;
    }

    sidebar->setSavePath(testDirStr);

    auto processEventsFor = [](int msec) {
        QEventLoop loop;
        QTimer::singleShot(msec, &loop, &QEventLoop::quit);
        loop.exec();
    };

    // Helper to count leftover temp files in testDir
    auto countTempFiles = [&]() {
        QStringList filters = {"*.part", "*.ytdl", "*.temp", "*.Frag*", "*.f[0-9]*"};
        return testDir.entryInfoList(filters, QDir::Files).size();
    };

    // TEST 1: Video download -> Pause -> Resume -> finish via MainWindow slots
    std::cout << "\n[TEST 1] Video download -> Pause -> Resume -> Finish via MainWindow slots..." << std::endl;
    mainPanel->setUrlText("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    window.onStartDownload(DownloadMode::Video);
    processEventsFor(2000);

    std::cout << "  Triggering onPauseRequested()..." << std::endl;
    window.onPauseRequested();
    processEventsFor(2000);

    std::cout << "  Triggering onPauseRequested() (Resume)..." << std::endl;
    window.onPauseRequested();
    processEventsFor(10000);

    std::cout << "  [TEST 1 RESULT]: Video Pause/Resume completed." << std::endl;

    // TEST 2: Video download -> Pause -> Cancel via MainWindow slots
    std::cout << "\n[TEST 2] Video download -> Pause -> Cancel via MainWindow slots..." << std::endl;
    mainPanel->setUrlText("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    window.onStartDownload(DownloadMode::Video);
    processEventsFor(2000);

    std::cout << "  Triggering onPauseRequested()..." << std::endl;
    window.onPauseRequested();
    processEventsFor(1000);

    std::cout << "  Triggering onCancelRequested()..." << std::endl;
    window.onCancelRequested();
    processEventsFor(1000);

    int leftover2 = countTempFiles();
    std::cout << "  [TEST 2 RESULT]: Leftover temp files after Pause->Cancel: " << leftover2 << std::endl;

    // TEST 3: MP3 download -> Cancel mid-conversion via MainWindow slots
    std::cout << "\n[TEST 3] MP3 download -> Cancel mid-conversion via MainWindow slots..." << std::endl;
    mainPanel->setUrlText("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    window.onStartDownload(DownloadMode::Audio);
    processEventsFor(1500);

    std::cout << "  Triggering onCancelRequested()..." << std::endl;
    window.onCancelRequested();
    processEventsFor(1000);

    int leftover3 = countTempFiles();
    std::cout << "  [TEST 3 RESULT]: Leftover temp files after MP3 Cancel: " << leftover3 << std::endl;

    // TEST 4: Rapid-fire start/cancel 5x through MainWindow slots
    std::cout << "\n[TEST 4] Rapid-fire 5x Start/Cancel cycles through MainWindow slots..." << std::endl;
    bool rapidSuccess = true;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "  Rapid cycle " << i << "..." << std::endl;
        mainPanel->setUrlText("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
        window.onStartDownload(DownloadMode::Video);
        processEventsFor(400);
        window.onCancelRequested();
        processEventsFor(300);
        if (countTempFiles() > 0) {
            rapidSuccess = false;
            std::cerr << "  FAIL: Leftover files after rapid cycle " << i << std::endl;
        }
    }
    std::cout << "  [TEST 4 RESULT]: Rapid-fire 5x start/cancel success: " << (rapidSuccess ? "PASS" : "FAIL") << std::endl;

    // TEST 5: Video + Crop and Audio + Crop through MainWindow UI inputs
    std::cout << "\n[TEST 5] Video + Crop (00:00:05 to 00:00:15) through MainWindow slots..." << std::endl;
    mainPanel->setUrlText("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    mainPanel->setCropTime("00:00:05", "00:00:15");
    window.onStartDownload(DownloadMode::Video);
    processEventsFor(12000);

    int leftover5 = countTempFiles();
    std::cout << "  [TEST 5 RESULT]: Leftover temp files after Crop Video download: " << leftover5 << std::endl;

    std::cout << "\n=========================================================" << std::endl;
    std::cout << "  ALL REAL APP C++/Qt UI INTEGRATION TESTS COMPLETED    " << std::endl;
    std::cout << "=========================================================" << std::endl;

    return 0;
}
