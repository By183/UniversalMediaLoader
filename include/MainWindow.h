#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "SidebarWidget.h"
#include "MainPanel.h"
#include "ProcessRunner.h"
#include "EngineUpdater.h"
#include "CommandBuilder.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void setTheme(bool isDark);
    void onSelectFolder();
    void onOpenFolder();
    void onStartDownload(DownloadMode mode);
    void onPauseRequested();
    void onCancelRequested();
    void onUpdateEngineRequested();
    void onPauseStateChanged(bool isPaused);
    void onProgressUpdated(const ParsedProgress &progress);
    void onLogEmitted(const QString &rawLine);
    void onProcessFinished(bool success, int exitCode);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QString getConfigPath() const;
    void loadConfig();
    void saveConfig();

    bool m_isDark;
    QString m_lastStatusMessage;
    QWidget *m_centralWidget;
    SidebarWidget *m_sidebar;
    MainPanel *m_mainPanel;
    ProcessRunner *m_runner;
    EngineUpdater *m_updater;
};

#endif // MAINWINDOW_H
