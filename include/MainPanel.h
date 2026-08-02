#ifndef MAINPANEL_H
#define MAINPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QLabel>
#include <QPlainTextEdit>
#include <QCheckBox>
#include "ProgressParser.h"
#include "CommandBuilder.h"

class MainPanel : public QWidget {
    Q_OBJECT

public:
    explicit MainPanel(QWidget *parent = nullptr);

    QString urlText() const;
    void setUrlText(const QString &url);
    QString customNameText() const;
    QString selectedQuality() const;
    QString timeStartText() const;
    QString timeEndText() const;
    void setCropTime(const QString &start, const QString &end);

    void setStatus(const QString &status);
    void setProgress(int percent, bool isIndeterminate = false);
    void appendLog(const QString &message);
    void clearLog();
    void setGuiDownloadingState(bool downloading);
    void setPausedState(bool isPaused);
    void parseAndSetTimestampFromUrl(const QString &url);

    QPushButton* downloadButton() const { return m_downloadBtn; }
    QPushButton* audioOnlyButton() const { return m_audioOnlyBtn; }
    QPushButton* thumbnailButton() const { return m_thumbnailBtn; }
    QPushButton* pauseButton() const { return m_pauseBtn; }
    QPushButton* cancelButton() const { return m_cancelBtn; }

signals:
    void downloadRequested(DownloadMode mode);
    void pauseRequested();
    void cancelRequested();

private slots:
    void onPasteClicked();
    void onCropToggled(bool checked);

private:
    QLineEdit *m_urlInput;
    QPushButton *m_pasteBtn;
    QLineEdit *m_nameInput;
    QComboBox *m_qualityCombo;
    QPushButton *m_thumbnailBtn;
    QCheckBox *m_cropCheckBox;
    QWidget *m_cropContainer;
    QLineEdit *m_timeStartInput;
    QLineEdit *m_timeEndInput;
    QPushButton *m_audioOnlyBtn;
    QPushButton *m_downloadBtn;
    QPushButton *m_pauseBtn;
    QPushButton *m_cancelBtn;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QPlainTextEdit *m_logEdit;
};

#endif // MAINPANEL_H
