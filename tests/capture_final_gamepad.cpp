#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <QDir>
#include <iostream>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.resize(1024, 620);
    window.show();

    QTimer::singleShot(1000, [&window, &app]() {
        QPixmap screenshot = window.grab();
        QString artifactPath = "C:/Users/Admin/.gemini/antigravity/brain/a70b4092-f1e5-425f-8409-6070662b86c9/.tempmediaStorage/gamepad_vector_preview.png";
        QDir().mkpath("C:/Users/Admin/.gemini/antigravity/brain/a70b4092-f1e5-425f-8409-6070662b86c9/.tempmediaStorage");
        if (screenshot.save(artifactPath)) {
            std::cout << "SCREENSHOT_SAVED: " << artifactPath.toStdString() << std::endl;
        }
        app.quit();
    });

    return app.exec();
}
