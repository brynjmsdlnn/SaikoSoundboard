#include "storage/StoragePaths.h"

#include <QDir>
#include <QStandardPaths>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

QString StoragePaths::settingsFilePath()
{
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    ensureDirectoryExists(appData);
    return appData + QStringLiteral("/settings.json");
}

// ---------------------------------------------------------------------------
// Default user-visible directories
// ---------------------------------------------------------------------------

QString StoragePaths::defaultBaseDirectory()
{
    return QDir::toNativeSeparators(QDir::homePath() + QStringLiteral("/Saiko Soundboard"));
}

QString StoragePaths::defaultRecordingDirectory()
{
    return QDir::toNativeSeparators(defaultBaseDirectory() + QStringLiteral("/recordings"));
}

QString StoragePaths::defaultReplayDirectory()
{
    return QDir::toNativeSeparators(defaultBaseDirectory() + QStringLiteral("/replays"));
}

void StoragePaths::ensureParentDirectoryExists(const QString &filePath)
{
    const int lastSlash = filePath.lastIndexOf(QChar('/'));
    if (lastSlash > 0) {
        QDir().mkpath(filePath.left(lastSlash));
    }
}

void StoragePaths::ensureDirectoryExists(const QString &dirPath)
{
    QDir().mkpath(dirPath);
}

// ---------------------------------------------------------------------------
// Temporary files
// ---------------------------------------------------------------------------

QString StoragePaths::temporaryDirectory()
{
    const QString dir = QDir::tempPath() + QStringLiteral("/Saiko Soundboard");
    ensureDirectoryExists(dir);
    return dir;
}

QString StoragePaths::temporaryReplayFilePath(const QString &timestamp)
{
    return temporaryDirectory() + QStringLiteral("/Replay_%1.wav").arg(timestamp);
}

bool StoragePaths::isTemporaryPath(const QString &filePath)
{
    // Pure query — no side effects. Resolve the path directly without
    // calling temporaryDirectory() to avoid unnecessary mkdir syscalls.
    const QString tempDir = QDir::tempPath() + QStringLiteral("/Saiko Soundboard");
    return filePath.startsWith(tempDir);
}

// ---------------------------------------------------------------------------
// Future — log storage
// ---------------------------------------------------------------------------

QString StoragePaths::logDirectory()
{
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appData + QStringLiteral("/logs");
}
