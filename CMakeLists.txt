cmake_minimum_required(VERSION 3.16)

project(SaikoSoundboard VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Workaround: MinGW 13.1 g++ -dM -E produces no output,
# causing AUTOMOC moc_predefs.h generation to fail.
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Multimedia Concurrent Quick Qml Test Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia Concurrent Quick Qml Test Svg)

enable_testing()

# --- Logging ---

add_subdirectory(src/logging)

# --- Storage ---

add_subdirectory(src/storage)

# --- Source Groups ---

set(CORE_DOMAIN_SOURCES
    src/core/domain/RingBuffer.h
    src/core/domain/KeyBindingStore.h
    src/core/domain/PlayerAllocator.h
)

set(CORE_ADAPTERS_SOURCES
    src/core/adapters/WindowsHotkeyBackend.h
    src/core/adapters/WindowsHotkeyBackend.cpp
    src/core/adapters/WindowsProcessFinder.h
    src/core/adapters/WindowsProcessFinder.cpp
    src/core/adapters/WindowsAudioSessionController.h
    src/core/adapters/WindowsAudioSessionController.cpp
)

set(UI_SOURCES
    src/ui/qmlbackend.cpp
    src/ui/qmlbackend.h
    src/ui/waveformitem.h
    src/ui/waveformitem.cpp
    src/ui/realtimewaveformitem.h
    src/ui/realtimewaveformitem.cpp
    src/ui/colorediconprovider.h
    src/ui/colorediconprovider.cpp
    src/ui/fileiconprovider.h
    src/ui/fileiconprovider.cpp
)

set(AUDIO_SOURCES
    src/audio/wasapirecorder.cpp
    src/audio/wasapirecorder.h
    src/audio/wasapipassthrough.cpp
    src/audio/wasapipassthrough.h
    src/audio/audiomixer.cpp
    src/audio/audiomixer.h
    src/audio/replaybuffer.cpp
    src/audio/replaybuffer.h
    src/audio/wavwriter.cpp
    src/audio/wavwriter.h
    src/audio/soundplayer.cpp
    src/audio/soundplayer.h
    src/audio/waveformgenerator.cpp
    src/audio/waveformgenerator.h
)

set(MANAGER_SOURCES
    src/managers/actionmanager.cpp
    src/managers/actionmanager.h
    src/managers/hotkeymanager.cpp
    src/managers/hotkeymanager.h
    src/managers/recordingmanager.cpp
    src/managers/recordingmanager.h
    src/managers/settingsmanager.cpp
    src/managers/settingsmanager.h
    src/managers/soundboardmanager.cpp
    src/managers/soundboardmanager.h
)

set(MODEL_SOURCES
    src/models/audiosource.h
    src/models/capturestate.h
    src/models/soundplayerslot.h
    src/models/soundplayerslotmodel.cpp
    src/models/soundplayerslotmodel.h
    src/models/audiosourcelistmodel.cpp
    src/models/audiosourcelistmodel.h
)

# --- Layer Libraries ---

# 1. Domain (Pure C++ business logic)
add_library(SaikoDomain STATIC ${CORE_DOMAIN_SOURCES})
target_include_directories(SaikoDomain PUBLIC src src/core src/core/domain)

# 2. Adapters (Windows specific interfaces)
add_library(SaikoAdapters STATIC ${CORE_ADAPTERS_SOURCES})
target_include_directories(SaikoAdapters PRIVATE src/core/adapters)
target_link_libraries(SaikoAdapters 
    PUBLIC 
        SaikoDomain
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        ole32
        mmdevapi
        shell32
)

# 3. Audio Engine
add_library(SaikoAudio STATIC ${AUDIO_SOURCES})
target_link_libraries(SaikoAudio 
    PUBLIC 
        SaikoDomain
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::Concurrent
)
set_target_properties(SaikoAudio PROPERTIES UNITY_BUILD ON)

# 4. Data Models (Value types only, header-only interface)
add_library(SaikoModels INTERFACE)
target_include_directories(SaikoModels INTERFACE src)
target_link_libraries(SaikoModels INTERFACE 
    SaikoDomain 
    SaikoLogging
    Qt${QT_VERSION_MAJOR}::Qml
)

# 5. Orchestration (Managers + Model QAbstractListModel classes)
add_library(SaikoManagers STATIC 
    ${MANAGER_SOURCES}
    ${MODEL_SOURCES}
)
target_link_libraries(SaikoManagers 
    PUBLIC 
        SaikoDomain
        SaikoAdapters
        SaikoAudio
        SaikoModels
        SaikoStorage
)
set_target_properties(SaikoManagers PROPERTIES UNITY_BUILD ON)

# 6. UI Backend
add_library(SaikoUI STATIC ${UI_SOURCES})
target_link_libraries(SaikoUI 
    PUBLIC 
        SaikoManagers
        SaikoModels
        SaikoAudio
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Svg
)

# Umbrella Core target to maintain compatibility with tests and tools
add_library(SaikoSoundboardCore INTERFACE)
target_link_libraries(SaikoSoundboardCore INTERFACE
    SaikoUI
    SaikoManagers
    SaikoModels
    SaikoAudio
    SaikoAdapters
    SaikoDomain
    SaikoLogging
    SaikoStorage
)

# --- Precompiled Headers ---

set(COMMON_PCH_HEADERS
    <vector>
    <string>
    <memory>
    <optional>
    <QObject>
    <QString>
    <QList>
    <QMap>
    <QHash>
    <QDebug>
)

if(WIN32)
    list(APPEND COMMON_PCH_HEADERS <windows.h>)
endif()

target_precompile_headers(SaikoAdapters PRIVATE ${COMMON_PCH_HEADERS})
target_precompile_headers(SaikoAudio PRIVATE ${COMMON_PCH_HEADERS})
target_precompile_headers(SaikoManagers PRIVATE ${COMMON_PCH_HEADERS})
target_precompile_headers(SaikoUI PRIVATE ${COMMON_PCH_HEADERS})

# --- Resource files ---

set(ICONS_RESOURCE resources/icons/icons.qrc)

# --- Platform-specific sources ---

set(PLATFORM_SOURCES)
if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/platform/WindowMetrics.h
        src/platform/windows/WindowsFramelessWindow.cpp
        src/platform/windows/WindowsFramelessWindow.h
    )
endif()

# --- Application Target ---

set(APP_SOURCES
    src/main.cpp
    ${PLATFORM_SOURCES}
    ${ICONS_RESOURCE}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SaikoSoundboard
        MANUAL_FINALIZATION
        ${APP_SOURCES}
    )
else()
    if(ANDROID)
        add_library(SaikoSoundboard SHARED
            ${APP_SOURCES}
        )
    else()
        add_executable(SaikoSoundboard
            ${APP_SOURCES}
        )
    endif()
endif()

target_include_directories(SaikoSoundboard PRIVATE src src/ui)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

set_source_files_properties(src/qml/shared/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(SaikoSoundboard
    URI Saiko
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml

        # --- Shared singletons & utilities ---
        src/qml/shared/Theme.qml
        src/qml/shared/utils.js

        # --- Reusable UI components ---
        src/qml/components/SaikoButton.qml
        src/qml/components/SaikoCheckBox.qml
        src/qml/components/SaikoComboBox.qml
        src/qml/components/SaikoDialog.qml
        src/qml/components/SaikoIconButton.qml
        src/qml/components/SaikoIconMenu.qml
        src/qml/components/SaikoTooltip.qml
        src/qml/components/SaikoMenu.qml
        src/qml/components/SaikoMenuItem.qml
        src/qml/components/SaikoSectionLabel.qml
        src/qml/components/LogViewer.qml
        src/qml/components/TitleBar.qml
        src/qml/components/WindowResizeHandlers.qml

        # --- Dialogs ---
        src/qml/dialogs/SettingsDialog.qml
        src/qml/dialogs/HotkeyDialog.qml
        src/qml/dialogs/RoutingDialog.qml
        src/qml/dialogs/SourceSelectionPopup.qml
        src/qml/dialogs/AboutDialog.qml
        src/qml/dialogs/AssignToSlotDialog.qml
        src/qml/dialogs/LogWindow.qml

        # --- Recording panel ---
        src/qml/recording/RecordingPanel.qml
        src/qml/recording/RecordingHeader.qml
        src/qml/recording/RecordingSection.qml
        src/qml/recording/ReplayBufferSection.qml
        src/qml/recording/ReplayDurationEditor.qml

        # --- Soundboard ---
        src/qml/soundboard/SoundboardGridView.qml
        src/qml/soundboard/SoundboardGridCard.qml
        src/qml/soundboard/SlotEditor.qml
        src/qml/soundboard/HotkeyCard.qml
        src/qml/soundboard/editor/EditorHeader.qml
        src/qml/soundboard/editor/AudioSourceSelector.qml
        src/qml/soundboard/editor/TrimEditor.qml
        src/qml/soundboard/editor/VolumeSlider.qml
        src/qml/soundboard/editor/RoutingSelector.qml
        src/qml/soundboard/editor/HotkeyDisplay.qml
        src/qml/soundboard/editor/ActionButtons.qml

        # --- Sources ---
        src/qml/sources/SourcesPanel.qml
        src/qml/sources/WaveformView.qml
)

target_link_libraries(SaikoSoundboard PRIVATE SaikoSoundboardCore)

if(WIN32)
    target_link_libraries(SaikoSoundboard PRIVATE dwmapi)
endif()

set_target_properties(SaikoSoundboard PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# --- Tests ---

add_subdirectory(tests)

# --- Tools ---

add_subdirectory(tools)

# --- Installation ---

include(GNUInstallDirs)
install(TARGETS SaikoSoundboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SaikoSoundboard)
endif()

# --- Packaging (CPack) ---

set(CPACK_PACKAGE_NAME "SaikoSoundboard")
set(CPACK_PACKAGE_VENDOR "Saiko Interactive")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Low-Latency Windows Soundboard & WASAPI Audio Recorder")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SaikoSoundboard")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_PACKAGE_EXECUTABLES "SaikoSoundboard" "SaikoSoundboard")
endif()

include(CPack)

