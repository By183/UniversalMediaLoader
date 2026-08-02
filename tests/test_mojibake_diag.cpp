#include <QCoreApplication>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDir>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== DIAGNOSING MOJIBAKE WITH CYRILLIC FOLDER PATH D:\\тест проги ===" << std::endl;

    QString outputDir = "D:/тест проги";
    QDir().mkpath(outputDir);

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    process.setProcessEnvironment(env);

    QString program = "D:/Universal Media Loader1/video_downloader/yt-dlp.exe";
    QStringList args;
    args << "--newline" << "--progress"
         << "-o" << (outputDir + "/TestFile.%(ext)s")
         << "--download-sections" << "*00:00:00-0:02"
         << "https://www.youtube.com/watch?v=aqz-KE-bpKQ";

    QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process]() {
        QByteArray data = process.readAllStandardOutput();
        QString utf8Str = QString::fromUtf8(data);
        QString localStr = QString::fromLocal8Bit(data);

        std::cout << "RAW BYTES COUNT: " << data.size() << std::endl;
        std::cout << "  [fromUTF8]:     " << utf8Str.toStdString();
        std::cout << "  [fromLocal8Bit]:" << localStr.toStdString();
    });

    process.start(program, args);
    process.waitForFinished(30000);

    return 0;
}
