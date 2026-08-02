#include <QApplication>
#include <QPixmap>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();
    window.setTheme(false); // Switch to Light Theme

    MainPanel *panel = window.findChild<MainPanel*>();
    if (panel) {
        QCheckBox *cb = panel->findChild<QCheckBox*>();
        if (cb) {
            cb->setChecked(true); // Expand time crop section
        }
    }

    QTimer::singleShot(500, [&window, &app]() {
        QPixmap screenshot = window.grab();
        QString artifactPath = "C:/Users/Admin/.gemini/antigravity/brain/a70b4092-f1e5-425f-8409-6070662b86c9/.user_uploaded/media__light_theme_styled_crop.png";
        screenshot.save(artifactPath, "PNG");
        std::cout << "SCREENSHOT SAVED TO: " << artifactPath.toStdString() << std::endl;
        app.quit();
    });

    return app.exec();
}
