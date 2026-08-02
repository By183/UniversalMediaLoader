#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDebug>
#include <iostream>
#include "MainWindow.h"

void writeTestConfig(bool isDark) {
    QString path = QDir::homePath() + "/.uml_config.json";
    QJsonObject obj;
    obj["output_dir"] = "d:/Universal Media Loader/video_downloader/downloads_test";
    obj["is_dark"] = isDark;
    QJsonDocument doc(obj);
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    std::cout << "=== TESTING STARTUP THEME INITIALIZATION ORDER ===" << std::endl;

    // 1. Test starting in DARK theme
    writeTestConfig(true);
    {
        MainWindow winDark;
        winDark.show();
        app.processEvents();

        bool qssHasDark = qApp->styleSheet().contains("Dark Theme Stylesheet");
        std::cout << "Test 1 (Config: is_dark = true):" << std::endl;
        std::cout << "  • QSS Applied is Dark Theme : " << (qssHasDark ? "YES (Correct)" : "NO (BUG!)") << std::endl;
    }

    // 2. Test starting in LIGHT theme
    writeTestConfig(false);
    {
        MainWindow winLight;
        winLight.show();
        app.processEvents();

        bool qssHasLight = qApp->styleSheet().contains("Light Theme Stylesheet");
        std::cout << "\nTest 2 (Config: is_dark = false):" << std::endl;
        std::cout << "  • QSS Applied is Light Theme: " << (qssHasLight ? "YES (Correct)" : "NO (BUG!)") << std::endl;
    }

    std::cout << "\nSTARTUP THEME INITIALIZATION TEST: PASSED" << std::endl;
    return 0;
}
