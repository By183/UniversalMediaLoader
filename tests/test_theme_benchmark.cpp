#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <iostream>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();
    app.processEvents();

    std::cout << "=== BENCHMARKING THEME TOGGLE PERFORMANCE ===" << std::endl;

    QElapsedTimer tGrab;
    tGrab.start();
    QPixmap pm = window.grab();
    qint64 grabMs = tGrab.elapsed();

    QElapsedTimer tStyle;
    tStyle.start();
    QFile file(":/styles/light.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
    qint64 styleMs = tStyle.elapsed();

    std::cout << "RESULT:" << std::endl;
    std::cout << "  • QWidget::grab() cost      : " << grabMs << " ms" << std::endl;
    std::cout << "  • qApp->setStyleSheet() cost: " << styleMs << " ms" << std::endl;

    return 0;
}
