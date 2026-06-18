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
