#include "managers/hotkeymanager.h"
#include "managers/actionmanager.h"
#include "managers/soundboardmanager.h"
#include "managers/recordingmanager.h"
#include "managers/settingsmanager.h"
#include "core/adapters/WindowsHotkeyBackend.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

void verifyHotkeyManager() {
    qDebug() << "Starting HotkeyManager verification...";

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
        qDebug() << "Registration SUCCESS for" << testKey;
    } else {
        qDebug() << "Registration FAILED for" << testKey;
    }

    // Test Duplicate Detection
    if (!hotkeyManager.registerHotkey(testKey, testAction)) {
        qDebug() << "Duplicate detection SUCCESS (prevented double registration)";
    } else {
        qDebug() << "Duplicate detection FAILED (allowed double registration)";
    }

    // Test IsRegistered
    if (hotkeyManager.isRegistered(testKey)) {
        qDebug() << "isRegistered SUCCESS";
    } else {
        qDebug() << "isRegistered FAILED";
    }

    // Test Unregistration
    hotkeyManager.unregisterHotkey(testKey);
    if (!hotkeyManager.isRegistered(testKey)) {
        qDebug() << "Unregistration SUCCESS";
    } else {
        qDebug() << "Unregistration FAILED";
    }

    qDebug() << "Verification complete. Note: Global hotkey trigger requires manual key press during app run.";
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    verifyHotkeyManager();
    // In a real test we'd run the loop to wait for events, 
    // but here we just verify the registration logic.
    return 0;
}
