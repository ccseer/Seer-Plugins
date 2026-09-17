cmake_minimum_required(VERSION 3.21)
project(explorer_share LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CPPWINRT_INCLUDE_DIR OR NOT EXISTS "${CPPWINRT_INCLUDE_DIR}/winrt/base.h")
    message(FATAL_ERROR "CPPWINRT_INCLUDE_DIR must be set to a directory containing winrt/base.h")
endif()

include(CTest)
enable_testing()

add_library(share_core STATIC
    src/share.cpp
    src/sharefile.h
)
target_include_directories(share_core PUBLIC src ${CPPWINRT_INCLUDE_DIR})
if(MSVC)
    target_compile_definitions(share_core PUBLIC
        _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
endif()
target_include_directories(share_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common)

target_link_libraries(share_core PUBLIC windowsapp ole32 user32 shell32)

add_executable(seer_share WIN32
    src/main.cpp
)
target_link_libraries(seer_share PRIVATE share_core)

add_executable(share_test
    test/share_test.cpp
)
target_link_libraries(share_test PRIVATE share_core)
add_test(NAME share_test COMMAND share_test)

set(SHARE_MANIFEST_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/manifest-stage)
add_custom_command(
    OUTPUT
        ${SHARE_MANIFEST_STAGING_DIR}/plugin.json
        ${SHARE_MANIFEST_STAGING_DIR}/seer_share.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHARE_MANIFEST_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${SHARE_MANIFEST_STAGING_DIR}/plugin.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:seer_share>
        ${SHARE_MANIFEST_STAGING_DIR}/seer_share.exe
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        seer_share
    VERBATIM
)
add_custom_target(share_manifest_stage
    DEPENDS
        ${SHARE_MANIFEST_STAGING_DIR}/plugin.json
        ${SHARE_MANIFEST_STAGING_DIR}/seer_share.exe
)

add_executable(share_manifest_test test/manifest_test.cpp)
add_dependencies(share_manifest_test share_manifest_stage)
add_test(NAME share_manifest_test COMMAND share_manifest_test ${SHARE_MANIFEST_STAGING_DIR})

install(TARGETS seer_share RUNTIME DESTINATION .)
install(FILES plugin.json DESTINATION .)
