#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QDir>
#include <iostream>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();
    window.resize(1050, 720);

    QString artifactDir = "C:/Users/Admin/.gemini/antigravity/brain/a70b4092-f1e5-425f-8409-6070662b86c9/.tempmediaStorage";
    QDir().mkpath(artifactDir);

    QTimer::singleShot(500, [&window, artifactDir]() {
        QPixmap pixmap = window.grab();
        QString savePath = artifactDir + "/thumbnail_button_ui.png";
        pixmap.save(savePath, "PNG");
        std::cout << "  [UI SCREENSHOT SAVED] " << savePath.toStdString() << std::endl;
        QTimer::singleShot(200, []() { QApplication::quit(); });
    });

    return app.exec();
}
