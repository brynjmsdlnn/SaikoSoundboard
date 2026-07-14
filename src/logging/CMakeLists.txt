# ------------------------------------------------------------------
# SaikoLogging — lightweight logging foundation
#
# Logger singleton dispatches to ConsoleSink, FileSink, and LogModel.
# No backend abstraction, no dynamic registration.
# ------------------------------------------------------------------

add_library(SaikoLogging STATIC
    FileSink.cpp
    FileSink.h
    Logger.cpp
    Logger.h
    ConsoleSink.cpp
    ConsoleSink.h
    Logging.cpp
    Logging.h
    LogLevel.h
    LogCategory.h
    LogMacros.h
    LogRecord.h
    LogModel.cpp
    LogModel.h
)

target_include_directories(SaikoLogging
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SaikoLogging
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
        SaikoStorage
)
