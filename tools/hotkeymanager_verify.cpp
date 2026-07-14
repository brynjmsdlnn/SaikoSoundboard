#include "managers/hotkeymanager.h"
#include "managers/actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"
#include "core/adapters/WindowsHotkeyBackend.h"
#include "logging/LogMacros.h"
#include <QCoreApplication>
#include <QTimer>

void verifyHotkeyManager() {
    LOG_DEBUG(LogCategory::General, QStringLiteral("Starting HotkeyManager verification..."));

    SettingsManager settings;
    SoundboardManager sb(&settings);
    RecordingManager rec(&settings);
    ActionManager actionManager(&sb, &rec, &settings);
    Saiko::Adapters::WindowsHotkeyBackend backend;
    HotkeyManager hotkeyManager(&actionManager, &backend);

    bool activated = false;
    QString activatedKey;
    QObject::connect(&hotkeyManager, &HotkeyManager::hotkeyActivated, [&](const QString &key){
        activated = true;
        activatedKey = key;
    });

    bool duplicateDetected = false;
    QObject::connect(&hotkeyManager, &HotkeyManager::duplicateDetected, [&](){
        duplicateDetected = true;
    });

    // Test Registration
    QString testKey = "Ctrl+Shift+T";
    Action testAction = Action::createPlay("test_id");

    if (hotkeyManager.registerHotkey(testKey, testAction)) {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Registration SUCCESS for %1").arg(testKey));
    } else {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Registration FAILED for %1").arg(testKey));
    }

    // Test Duplicate Detection
    if (!hotkeyManager.registerHotkey(testKey, testAction)) {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Duplicate detection SUCCESS (prevented double registration)"));
    } else {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Duplicate detection FAILED (allowed double registration)"));
    }

    // Test IsRegistered
    if (hotkeyManager.isRegistered(testKey)) {
        LOG_DEBUG(LogCategory::General, QStringLiteral("isRegistered SUCCESS"));
    } else {
        LOG_DEBUG(LogCategory::General, QStringLiteral("isRegistered FAILED"));
    }

    // Test Unregistration
    hotkeyManager.unregisterHotkey(testKey);
    if (!hotkeyManager.isRegistered(testKey)) {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Unregistration SUCCESS"));
    } else {
        LOG_DEBUG(LogCategory::General, QStringLiteral("Unregistration FAILED"));
    }

    LOG_DEBUG(LogCategory::General, QStringLiteral("Verification complete. Note: Global hotkey trigger requires manual key press during app run."));
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    verifyHotkeyManager();
    return 0;
}
