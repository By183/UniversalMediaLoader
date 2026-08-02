#include <QApplication>
#include <QIcon>
#include "MainWindow.h"
#include "EmbeddedToolsManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon.ico"));

    EmbeddedToolsManager::ensureToolsExtracted();

    MainWindow window;
    window.show();

    return app.exec();
}
