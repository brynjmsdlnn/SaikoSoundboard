#ifndef SAIKO_STORAGE_STORAGEPATHS_H
#define SAIKO_STORAGE_STORAGEPATHS_H

#include <QString>

// ---------------------------------------------------------------------------
// StoragePaths — single source of truth for all filesystem path construction.
//
// Responsibilities:
//   - Construct filesystem paths from logical names / timestamps
//   - Create parent directories when appropriate (mkpath)
//   - Answer filesystem-related questions (isTemporaryPath)
//
// Non-responsibilities:
//   - File I/O (read/write files, JSON, etc.)
//   - Application state, user overrides, settings
//   - Logging, audio, or QML knowledge
//
// This class is deliberately stateless. All methods are static.
// Future portable-mode detection should be implemented here and only here,
// without any other part of the application needing to know the storage layout.
// ---------------------------------------------------------------------------

class StoragePaths
{
public:
    // -----------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------

    // Full path to the settings.json file.
    // Uses QStandardPaths::AppDataLocation + "/settings.json".
    static QString settingsFilePath();

    // -----------------------------------------------------------------------
    // Default user-visible directories
    // -----------------------------------------------------------------------

    // The root base directory under which recordings/replays live by default.
    // Currently QDir::homePath() + "/Saiko Soundboard".
    static QString defaultBaseDirectory();

    // Default directory for recordings: defaultBaseDirectory() + "/recordings".
    static QString defaultRecordingDirectory();

    // Default directory for saved replays: defaultBaseDirectory() + "/replays".
    static QString defaultReplayDirectory();

    // Ensure the directory for the given file path exists.
    // Equivalent to QDir().mkpath() on the parent directory.
    static void ensureParentDirectoryExists(const QString &filePath);

    // Ensure the given directory exists.
    static void ensureDirectoryExists(const QString &dirPath);

    // -----------------------------------------------------------------------
    // Temporary files
    // -----------------------------------------------------------------------

    // Dedicated Saiko temporary directory under the system temp folder.
    // Pattern: <tempPath>/Saiko Soundboard
    // The directory is created automatically if it does not exist.
    static QString temporaryDirectory();

    // Generate a full path for a temporary replay in the Saiko temp directory.
    // Pattern: <temporaryDirectory()>/SaikoReplay_<timestamp>.wav
    static QString temporaryReplayFilePath(const QString &timestamp);

    // Returns true if the given file path lives under temporaryDirectory().
    static bool isTemporaryPath(const QString &filePath);

    // -----------------------------------------------------------------------
    // Future — log storage
    // -----------------------------------------------------------------------

    // Directory for application log files.
    // Currently QStandardPaths::AppDataLocation + "/logs".
    // Defined now so FileSink can use it without further architectural changes.
    static QString logDirectory();

private:
    StoragePaths() = delete; // stateless utility only
};

#endif // SAIKO_STORAGE_STORAGEPATHS_H
