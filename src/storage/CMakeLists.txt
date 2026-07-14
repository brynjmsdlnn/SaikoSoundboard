# SaikoStorage — lightweight filesystem path utility
#
# Depends only on QtCore. No knowledge of audio, logging, or UI.

add_library(SaikoStorage STATIC
    StoragePaths.cpp
    StoragePaths.h
)
target_include_directories(SaikoStorage PUBLIC ..)
target_link_libraries(SaikoStorage PUBLIC Qt${QT_VERSION_MAJOR}::Core)
