#include "ProcessRunner.h"
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QSet>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <windows.h>
#include <tlhelp32.h>

typedef LONG (NTAPI *pfnNtSuspendProcess)(HANDLE ProcessHandle);
typedef LONG (NTAPI *pfnNtResumeProcess)(HANDLE ProcessHandle);

static pfnNtSuspendProcess g_NtSuspendProcess = nullptr;
static pfnNtResumeProcess g_NtResumeProcess = nullptr;

static double parseTimeToSecondsHelper(const QString &tsStr) {
    if (tsStr.trimmed().isEmpty()) return -1.0;
    QString str = tsStr.trimmed();

    if (!str.contains(':')) {
        bool ok = false;
        double mins = str.toDouble(&ok);
        if (ok && mins >= 0) {
            return mins * 60.0;
        }
        return -1.0;
    }

    QStringList parts = str.split(':');
    if (parts.size() == 3) {
        return parts[0].toDouble() * 3600.0 + parts[1].toDouble() * 60.0 + parts[2].toDouble();
    }
    if (parts.size() == 2) {
        return parts[0].toDouble() * 60.0 + parts[1].toDouble();
    }
    return -1.0;
}

static void initNtApis() {
    if (!g_NtSuspendProcess) {
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            g_NtSuspendProcess = (pfnNtSuspendProcess)GetProcAddress(hNtdll, "NtSuspendProcess");
            g_NtResumeProcess = (pfnNtResumeProcess)GetProcAddress(hNtdll, "NtResumeProcess");
        }
    }
}

static QString decodeProcessOutput(const QByteArray &data) {
    QString utf8Str = QString::fromUtf8(data);
    if (!utf8Str.contains(QChar(0xFFFD))) {
        return utf8Str;
    }
    return QString::fromLocal8Bit(data);
}

static bool tryRemoveFileWithRetry(const QString &filePath) {
    if (!QFile::exists(filePath)) return true;

    for (int attempt = 1; attempt <= 10; ++attempt) {
        if (QFile::remove(filePath)) {
            return true;
        }
        QThread::msleep(150);
    }
    return false;
}

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent), m_process(new QProcess(this)), m_jobObject(nullptr), m_isPaused(false), m_isCancelled(false), m_isThumbnailMode(false)
{
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &ProcessRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &ProcessRunner::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ProcessRunner::onProcessFinished);
}

ProcessRunner::~ProcessRunner() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        cancel();
    }
    if (m_jobObject) {
        CloseHandle((HANDLE)m_jobObject);
        m_jobObject = nullptr;
    }
}

bool ProcessRunner::isRunning() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

qint64 ProcessRunner::processId() const {
    return m_process ? m_process->processId() : 0;
}

void ProcessRunner::startDownload(const QStringList &args, const QString &outputDir) {
    if (isRunning()) return;

    m_outputDir = outputDir;
    m_targetFilePath.clear();
    m_createdFiles.clear();
    m_startTime = QDateTime::currentDateTime();
    m_isPaused = false;
    m_isCancelled = false;
    m_isThumbnailMode = args.contains("--skip-download");
    m_aggregator.reset();

    bool isAudioOnly = args.contains("-x") || args.contains("--audio-format");
    m_aggregator.setAudioOnly(isAudioOnly);

    // Extract crop range directly from command arguments
    for (int i = 0; i < args.size(); ++i) {
        if (args[i] == "--download-sections" && i + 1 < args.size()) {
            QString spec = args[i + 1];
            static QRegularExpression rxSec(R"(\*(\S+)-(\S+))");
            QRegularExpressionMatch m = rxSec.match(spec);
            if (m.hasMatch()) {
                double start = parseTimeToSecondsHelper(m.captured(1));
                double end = parseTimeToSecondsHelper(m.captured(2));
                if (start >= 0 && end > start) {
                    m_aggregator.setCropRange(start, end);
                }
            }
        }
    }

    m_stdoutBuffer.clear();
    m_stderrBuffer.clear();

    if (args.isEmpty()) return;

    if (m_jobObject) {
        CloseHandle((HANDLE)m_jobObject);
        m_jobObject = nullptr;
    }

    HANDLE hJob = CreateJobObjectW(NULL, NULL);
    if (hJob) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
        m_jobObject = hJob;
    }

    int oIndex = args.indexOf("-o");
    if (oIndex >= 0 && oIndex + 1 < args.size()) {
        QString tmpl = args[oIndex + 1];
        if (!tmpl.contains("%(")) {
            m_targetFilePath = tmpl;
            m_createdFiles.insert(tmpl);
        }
    }

    QString program = args.first();
    QStringList arguments = args.mid(1);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    m_process->setProcessEnvironment(env);

    m_process->start(program, arguments);

    // Immediately close stdin so non-interactive subprocesses (ffmpeg) never wait for stdin
    m_process->closeWriteChannel();

    if (m_jobObject && m_process->waitForStarted(1000)) {
        HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(m_process->processId()));
        if (hProcess) {
            AssignProcessToJobObject((HANDLE)m_jobObject, hProcess);
            CloseHandle(hProcess);
        }
    }
}

bool ProcessRunner::pause() {
    if (!isRunning() || m_isPaused) return false;

    qint64 pid = processId();
    if (pid <= 0) return false;

    suspendProcessTree(static_cast<unsigned long>(pid));
    m_isPaused = true;
    emit pauseStateChanged(true);

    ParsedProgress prog;
    prog.statusMessage = "⏸  Загрузка приостановлена";
    emit progressUpdated(prog);

    return true;
}

bool ProcessRunner::resume() {
    if (!isRunning() || !m_isPaused) return false;

    qint64 pid = processId();
    if (pid <= 0) return false;

    resumeProcessTree(static_cast<unsigned long>(pid));
    m_isPaused = false;
    emit pauseStateChanged(false);

    ParsedProgress prog;
    prog.statusMessage = "▶  Загрузка возобновлена";
    emit progressUpdated(prog);

    return true;
}

void ProcessRunner::cancel() {
    if (!isRunning() && !m_isPaused) return;

    m_isCancelled = true;

    qint64 pid = processId();
    if (pid > 0) {
        resumeProcessTree(static_cast<unsigned long>(pid));
        QThread::msleep(50);
    }

    if (m_jobObject) {
        TerminateJobObject((HANDLE)m_jobObject, 1);
        CloseHandle((HANDLE)m_jobObject);
        m_jobObject = nullptr;
    }

    if (pid > 0) {
        killProcessTree(static_cast<unsigned long>(pid));
    }

    if (m_process->state() != QProcess::NotRunning) {
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }

    QThread::msleep(150);

    cleanupTempFiles();

    m_isPaused = false;
    emit pauseStateChanged(false);
}

void ProcessRunner::cleanupAudioCoverTempFiles() {
    if (m_outputDir.isEmpty()) return;
    QDir dir(m_outputDir);
    if (!dir.exists()) return;

    // Delete any leftover standalone image file (.jpg, .jpeg, .png, .webp) created during video/audio download sessions
    QStringList imageFilters = {"*.jpg", "*.jpeg", "*.png", "*.webp"};
    QFileInfoList imageFiles = dir.entryInfoList(imageFilters, QDir::Files);
    for (const QFileInfo &f : imageFiles) {
        if (!m_targetFilePath.isEmpty() && f.absoluteFilePath() == QFileInfo(m_targetFilePath).absoluteFilePath()) {
            continue;
        }
        // NEVER delete preview files explicitly downloaded by the user (_preview)
        if (f.fileName().contains("_preview", Qt::CaseInsensitive)) {
            continue;
        }
        if (m_startTime.isValid() && f.lastModified() >= m_startTime.addSecs(-15)) {
            tryRemoveFileWithRetry(f.absoluteFilePath());
        }
    }
}

void ProcessRunner::cleanupTempFiles() {
    if (m_outputDir.isEmpty()) return;
    QDir dir(m_outputDir);
    if (!dir.exists()) return;

    QSet<QString> filesToDelete;

    QStringList filters = {"*.part", "*.ytdl", "*.temp", "*.Frag*", "*.f[0-9]*"};
    for (const QString &filter : filters) {
        QFileInfoList files = dir.entryInfoList(QStringList() << filter, QDir::Files);
        for (const QFileInfo &fileInfo : files) {
            // NEVER delete preview files explicitly downloaded by the user
            if (fileInfo.fileName().contains("_preview", Qt::CaseInsensitive)) {
                continue;
            }
            filesToDelete.insert(fileInfo.absoluteFilePath());
        }
    }

    QSet<QString> candidatePaths = m_createdFiles;
    if (!m_targetFilePath.isEmpty()) candidatePaths.insert(m_targetFilePath);

    for (const QString &cPath : candidatePaths) {
        QFileInfo cInfo(cPath);
        QString baseName = cInfo.completeBaseName();
        int dotIdx = baseName.indexOf('.');
        if (dotIdx > 0) {
            baseName = baseName.left(dotIdx);
        }
        if (!baseName.isEmpty() && baseName.length() >= 2) {
            QFileInfoList matchedFiles = dir.entryInfoList(QStringList() << (baseName + "*"), QDir::Files);
            for (const QFileInfo &fileInfo : matchedFiles) {
                if (fileInfo.fileName().contains("_preview", Qt::CaseInsensitive)) {
                    continue;
                }
                filesToDelete.insert(fileInfo.absoluteFilePath());
            }
        }
    }

    if (m_startTime.isValid()) {
        QFileInfoList allFiles = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : allFiles) {
            if (fileInfo.fileName().contains("_preview", Qt::CaseInsensitive)) {
                continue;
            }
            if (fileInfo.lastModified() >= m_startTime.addSecs(-15)) {
                filesToDelete.insert(fileInfo.absoluteFilePath());
            }
        }
    }

    for (const QString &filePath : filesToDelete) {
        bool ok = tryRemoveFileWithRetry(filePath);
        if (ok) {
            emit logOutputEmitted(QString("🗑  Удален временный файл: %1").arg(QFileInfo(filePath).fileName()));
        } else {
            emit logOutputEmitted(QString("⚠  Не удалось удалить заблокированный файл: %1").arg(QFileInfo(filePath).fileName()));
        }
    }
}

void ProcessRunner::onReadyReadStandardOutput() {
    if (m_isPaused) return;
    QByteArray data = m_process->readAllStandardOutput();
    m_stdoutBuffer += decodeProcessOutput(data);
    processLineBuffer(m_stdoutBuffer, false);
}

void ProcessRunner::onReadyReadStandardError() {
    if (m_isPaused) return;
    QByteArray data = m_process->readAllStandardError();
    m_stderrBuffer += decodeProcessOutput(data);
    processLineBuffer(m_stderrBuffer, true);
}

void ProcessRunner::processLineBuffer(QString &buffer, bool isErrorStream) {
    int idx;
    while ((idx = buffer.indexOf('\n')) != -1 || (idx = buffer.indexOf('\r')) != -1) {
        QString line = buffer.left(idx).trimmed();
        buffer.remove(0, idx + 1);

        if (line.isEmpty()) continue;

        ParsedProgress progress = m_aggregator.processLine(line);

        if (progress.type == ProgressType::IgnoredLine) {
            continue;
        }

        emit logOutputEmitted(line);

        if (!progress.destinationFile.isEmpty()) {
            m_targetFilePath = progress.destinationFile;
            m_createdFiles.insert(progress.destinationFile);
        }

        if (isErrorStream && !progress.isError && progress.type == ProgressType::GenericInfo) {
            progress.type = ProgressType::Warning;
        }
        emit progressUpdated(progress);
    }
}

void ProcessRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (!m_stdoutBuffer.trimmed().isEmpty()) {
        processLineBuffer(m_stdoutBuffer, false);
    }
    if (!m_stderrBuffer.trimmed().isEmpty()) {
        processLineBuffer(m_stderrBuffer, true);
    }

    bool wasCancelled = m_isCancelled;
    m_isPaused = false;
    emit pauseStateChanged(false);

    if (wasCancelled) {
        cleanupTempFiles();
        emit finished(false, -1);
    } else {
        bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
        if (success && !m_isThumbnailMode) {
            // Clean up standalone image cover files created during audio/video downloads
            cleanupAudioCoverTempFiles();
        }
        emit finished(success, exitCode);
    }
}

void ProcessRunner::suspendProcessTree(unsigned long pid) {
    initNtApis();
    if (!g_NtSuspendProcess) return;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(pe);

    QList<DWORD> pids;
    pids.append(pid);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ParentProcessID == pid) {
                pids.append(pe.th32ProcessID);
            }
        } while (Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);

    for (DWORD p : pids) {
        HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, p);
        if (hProcess) {
            g_NtSuspendProcess(hProcess);
            CloseHandle(hProcess);
        }
    }
}

void ProcessRunner::resumeProcessTree(unsigned long pid) {
    initNtApis();
    if (!g_NtResumeProcess) return;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(pe);

    QList<DWORD> pids;
    pids.append(pid);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ParentProcessID == pid) {
                pids.append(pe.th32ProcessID);
            }
        } while (Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);

    for (DWORD p : pids) {
        HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, p);
        if (hProcess) {
            g_NtResumeProcess(hProcess);
            CloseHandle(hProcess);
        }
    }
}

void ProcessRunner::killProcessTree(unsigned long pid) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(pe);

    QList<DWORD> children;

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ParentProcessID == pid) {
                children.append(pe.th32ProcessID);
            }
        } while (Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);

    for (DWORD childPid : children) {
        killProcessTree(childPid);
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
    }
}
