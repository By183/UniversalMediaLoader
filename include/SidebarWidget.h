#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include "ToggleSwitch.h"

class SidebarWidget : public QWidget {
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    void setSavePath(const QString &path);
    QString savePath() const { return m_savePath; }
    void setUpdatingEngineState(bool updating);
    void setThemeChecked(bool isDark);

signals:
    void themeToggled(bool isDark);
    void selectFolderRequested();
    void openFolderRequested();
    void updateEngineRequested();

private:
    QFrame* createSeparator();
    QString shortenPath(const QString &path, int maxLength = 28);

    QString m_savePath;
    QLabel *m_pathLabel;
    QPushButton *m_btnUpdateEngine;
    ToggleSwitch *m_themeSwitch;
};

#endif // SIDEBARWIDGET_H
