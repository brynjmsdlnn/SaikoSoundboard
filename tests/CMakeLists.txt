# --- Unit Tests (Automated Qt Test) ---

macro(add_saiko_unit_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE 
        SaikoSoundboardCore 
        Qt${QT_VERSION_MAJOR}::Test
    )
    add_test(NAME ${NAME} COMMAND ${NAME})
endmacro()

add_saiko_unit_test(RingBufferUnitTest unit/ringbuffer_test.cpp)

# --- Integration Tests (Automated Qt Test) ---

macro(add_saiko_integration_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE 
        SaikoSoundboardCore 
        Qt${QT_VERSION_MAJOR}::Test
    )
    add_test(NAME ${NAME} COMMAND ${NAME})
endmacro()

add_saiko_integration_test(ActionManagerIntegrationTest integration/actionmanager_test.cpp)
