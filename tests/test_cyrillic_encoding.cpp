#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include "CommandBuilder.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING CYRILLIC ENCODING FIX FOR DISK FILENAME & STDOUT LOG ===" << std::endl;

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "D:/Universal Media Loader1/video_downloader/downloads_cyrillic_test";
    QDir().mkpath(outputDir);
    QString customName = "Тест_Українська_Мова_Скачування_Fix";

    QStringList args = CommandBuilder::buildCommand(url, "1080p", false, outputDir, customName, "0", "0:05");

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    process.setProcessEnvironment(env);

    QString program = args.first();
    QStringList arguments = args.mid(1);

    std::cout << "Launching command: " << program.toStdString() << std::endl;

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process]() {
        QByteArray data = process.readAllStandardOutput();
        QString str = QString::fromUtf8(data);
        std::cout << "[STDOUT UTF-8] " << str.toStdString();
    });

    QObject::connect(&process, &QProcess::readyReadStandardError, [&process]() {
        QByteArray data = process.readAllStandardError();
        QString str = QString::fromUtf8(data);
        std::cout << "[STDERR UTF-8] " << str.toStdString();
    });

    process.start(program, arguments);
    process.waitForFinished(60000);

    std::cout << "\nChecking files on disk in " << outputDir.toStdString() << ":" << std::endl;
    QDir dir(outputDir);
    QFileInfoList files = dir.entryInfoList(QStringList() << (customName + ".*"), QDir::Files);
    for (const QFileInfo &f : files) {
        std::cout << "  - DISK FILE: " << f.fileName().toStdString() << " (Length: " << f.size() << " bytes)" << std::endl;
    }

    return 0;
}
