#include "EmbeddedToolsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <windows.h>

QString EmbeddedToolsManager::getToolsDir() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString binDir = appData + "/bin";
    QDir().mkpath(binDir);
    return binDir;
}

bool EmbeddedToolsManager::extractResourceIfMissing(int resourceId, const QString &targetFilePath) {
    QFileInfo targetInfo(targetFilePath);

    HMODULE hModule = GetModuleHandleW(NULL);
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!hRes) {
        qDebug() << "[EMBEDDED_TOOLS] RCDATA Resource ID" << resourceId << "not found in executable (may be running un-embedded build).";
        return false;
    }

    DWORD resSize = SizeofResource(hModule, hRes);
    if (targetInfo.exists() && targetInfo.size() == static_cast<qint64>(resSize)) {
        qDebug() << "[EMBEDDED_TOOLS] Tool already extracted & size matches:" << targetFilePath << "(" << resSize << "bytes)";
        return true;
    }

    HGLOBAL hMem = LoadResource(hModule, hRes);
    if (!hMem) return false;

    void *pData = LockResource(hMem);
    if (!pData) return false;

    QFile outFile(targetFilePath);
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "[EMBEDDED_TOOLS] Failed to open target file for writing:" << targetFilePath;
        return false;
    }

    qint64 written = outFile.write(reinterpret_cast<const char*>(pData), resSize);
    outFile.close();

    qDebug() << "[EMBEDDED_TOOLS] Successfully extracted resource ID" << resourceId << "to" << targetFilePath << "(" << written << "bytes)";
    return (written == static_cast<qint64>(resSize));
}

bool EmbeddedToolsManager::ensureToolsExtracted() {
    QString dir = getToolsDir();
    QString ffmpegPath = dir + "/ffmpeg.exe";
    QString ffprobePath = dir + "/ffprobe.exe";

    bool okFfmpeg = extractResourceIfMissing(101, ffmpegPath);
    bool okFfprobe = extractResourceIfMissing(102, ffprobePath);

    return okFfmpeg && okFfprobe;
}
