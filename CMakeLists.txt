cmake_minimum_required(VERSION 3.16)

project(SaikoSoundboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Concurrent Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Concurrent Test)

enable_testing()

# --- Source Groups ---

set(UI_SOURCES
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/sourcesdock.cpp
    src/ui/sourcesdock.h
    src/ui/soundboarddock.cpp
    src/ui/soundboarddock.h
    src/ui/hotkeydialog.cpp
    src/ui/hotkeydialog.h
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
)

# --- Core Library ---

add_library(SaikoSoundboardCore STATIC
    ${AUDIO_SOURCES}
    ${MANAGER_SOURCES}
    ${MODEL_SOURCES}
)

target_include_directories(SaikoSoundboardCore PUBLIC src)
target_link_libraries(SaikoSoundboardCore PUBLIC 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Multimedia 
    Qt${QT_VERSION_MAJOR}::Concurrent
)

if(WIN32)
    target_link_libraries(SaikoSoundboardCore PUBLIC ole32 mmdevapi)
endif()

# --- Application Target ---

set(APP_SOURCES
    src/main.cpp
    ${UI_SOURCES}
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

target_link_libraries(SaikoSoundboard PRIVATE SaikoSoundboardCore)

set_target_properties(SaikoSoundboard PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# --- Tests ---

add_subdirectory(tests)

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
