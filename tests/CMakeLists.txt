# --- Options ---
option(BUILD_VERIFICATION_TOOLS "Build manual hardware/system verification tools" ON)

# --- Verification Executables (Manual Validation) ---

macro(add_saiko_verification NAME SOURCE)
    if(BUILD_VERIFICATION_TOOLS)
        add_executable(${NAME} ${SOURCE})
        target_link_libraries(${NAME} PRIVATE SaikoSoundboardCore)
    endif()
endmacro()

add_saiko_verification(SoundPlayerVerify verification/audio/soundplayer_verify.cpp)
add_saiko_verification(ActionManagerVerify verification/managers/actionmanager_verify.cpp)
add_saiko_verification(HotkeyManagerVerify verification/managers/hotkeymanager_verify.cpp)

# --- Unit Tests (Automated Qt Test) ---

macro(add_saiko_unit_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE 
        SaikoSoundboardCore 
        Qt${QT_VERSION_MAJOR}::Test
    )
    add_test(NAME ${NAME} COMMAND ${NAME})
endmacro()

# Future unit tests will be added here
add_saiko_unit_test(ActionManagerUnitTest unit/actionmanager_test.cpp)
add_saiko_unit_test(RingBufferUnitTest unit/ringbuffer_test.cpp)
