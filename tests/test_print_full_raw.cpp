#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <iostream>
#include "CommandBuilder.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== PRINTING ALL RAW STDOUT & STDERR FOR MP3+CROP ===" << std::endl;

    QString outputDir = "D:/test_crop_raw";
    QDir().mkpath(outputDir);

    QStringList args = CommandBuilder::buildCommand(
        "https://www.youtube.com/watch?v=aqz-KE-bpKQ",
        "1080p",
        true, // audioOnly
        outputDir,
        "PrintFullRawMp3",
        "00:00:15",
        "00:01:45"
    );

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    process.setProcessEnvironment(env);

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process]() {
        QByteArray data = process.readAllStandardOutput();
        std::cout << "[STDOUT RAW (" << data.size() << "b)] " << data.toStdString();
    });

    QObject::connect(&process, &QProcess::readyReadStandardError, [&process]() {
        QByteArray data = process.readAllStandardError();
        std::cout << "[STDERR RAW (" << data.size() << "b)] " << data.toStdString();
    });

    process.start(args.first(), args.mid(1));
    process.closeWriteChannel();

    process.waitForFinished(180000);
    std::cout << "\nFINISHED. EXIT CODE: " << process.exitCode() << std::endl;

    return 0;
}
