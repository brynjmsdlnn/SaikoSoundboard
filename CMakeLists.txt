cmake_minimum_required(VERSION 3.16)

project(SaikoSoundboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Workaround: MinGW 13.1 g++ -dM -E produces no output,
# causing AUTOMOC moc_predefs.h generation to fail.
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Multimedia Concurrent Quick Qml Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia Concurrent Quick Qml Test)

enable_testing()

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
)

set(UI_SOURCES
    src/ui/qmlbackend.cpp
    src/ui/qmlbackend.h
)

set(AUDIO_SOURCES
    src/audio/wasapirecorder.cpp
    src/audio/wasapirecorder.h
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
)

# --- Core Library ---

add_library(SaikoSoundboardCore STATIC
    ${CORE_DOMAIN_SOURCES}
    ${CORE_ADAPTERS_SOURCES}
    ${UI_SOURCES}
    ${AUDIO_SOURCES}
    ${MANAGER_SOURCES}
    ${MODEL_SOURCES}
)

target_include_directories(SaikoSoundboardCore PUBLIC src)
target_include_directories(SaikoSoundboardCore PUBLIC src/core)
target_include_directories(SaikoSoundboardCore PUBLIC src/core/domain)
target_include_directories(SaikoSoundboardCore PRIVATE src/core/adapters)
target_link_libraries(SaikoSoundboardCore PUBLIC 
    Qt${QT_VERSION_MAJOR}::Multimedia 
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
)

if(WIN32)
    target_link_libraries(SaikoSoundboardCore PUBLIC ole32 mmdevapi shell32)
endif()

# --- Application Target ---

set(APP_SOURCES
    src/main.cpp
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

qt_add_qml_module(SaikoSoundboard
    URI Saiko
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/HotkeyDialog.qml
        src/qml/SourcesPanel.qml
        src/qml/RoutingDialog.qml
        src/qml/WaveformView.qml
        src/qml/SoundboardCard.qml
        src/qml/SoundboardPanel.qml
        src/qml/CustomComboBox.qml
        src/qml/CustomCheckBox.qml
    SOURCES
        src/ui/waveformitem.h src/ui/waveformitem.cpp
        src/ui/realtimewaveformitem.h src/ui/realtimewaveformitem.cpp
)

target_link_libraries(SaikoSoundboard PRIVATE SaikoSoundboardCore)

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
