#ifndef EMBEDDEDTOOLSMANAGER_H
#define EMBEDDEDTOOLSMANAGER_H

#include <QString>

class EmbeddedToolsManager {
public:
    static QString getToolsDir();
    static bool ensureToolsExtracted();

private:
    static bool extractResourceIfMissing(int resourceId, const QString &targetFilePath);
};

#endif // EMBEDDEDTOOLSMANAGER_H
