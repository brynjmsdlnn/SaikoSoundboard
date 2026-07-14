# --- Tools ---
option(BUILD_VERIFICATION_TOOLS "Build manual hardware/system verification tools" ON)

if(BUILD_VERIFICATION_TOOLS)
    macro(add_saiko_verification NAME SOURCE)
        add_executable(${NAME} ${SOURCE})
        target_link_libraries(${NAME} PRIVATE SaikoSoundboardCore)
    endmacro()

    add_saiko_verification(SoundPlayerVerify soundplayer_verify.cpp)
    add_saiko_verification(ActionManagerVerify actionmanager_verify.cpp)
    add_saiko_verification(HotkeyManagerVerify hotkeymanager_verify.cpp)
endif()

# --- Architecture Verification (manual, never part of default build) ---

find_program(POWERSHELL_EXECUTABLE NAMES pwsh powershell)
if(POWERSHELL_EXECUTABLE)
    add_custom_target(verify-project
        COMMAND ${POWERSHELL_EXECUTABLE} -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_SOURCE_DIR}/tools/verify_project.ps1"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running architecture verification (logging rules, singleton rules, include rules)..."
    )
endif()
