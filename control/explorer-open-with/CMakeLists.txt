cmake_minimum_required(VERSION 3.21)
project(explorer_open_with LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(openwith_core STATIC src/openwith.cpp)
target_include_directories(openwith_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(openwith_core PUBLIC shell32)

add_executable(shellopenwith WIN32 src/main.cpp)
target_link_libraries(shellopenwith PRIVATE openwith_core shell32)

add_executable(shellopenwith_test test/openwith_test.cpp)
target_link_libraries(shellopenwith_test PRIVATE openwith_core)
add_test(NAME shellopenwith_test COMMAND shellopenwith_test)

set(OPENWITH_MANIFEST_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/manifest-stage)
add_custom_command(
    OUTPUT
        ${OPENWITH_MANIFEST_STAGING_DIR}/plugin.json
        ${OPENWITH_MANIFEST_STAGING_DIR}/shellopenwith.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OPENWITH_MANIFEST_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${OPENWITH_MANIFEST_STAGING_DIR}/plugin.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:shellopenwith>
        ${OPENWITH_MANIFEST_STAGING_DIR}/shellopenwith.exe
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        shellopenwith
    VERBATIM
)
add_custom_target(openwith_manifest_stage
    DEPENDS
        ${OPENWITH_MANIFEST_STAGING_DIR}/plugin.json
        ${OPENWITH_MANIFEST_STAGING_DIR}/shellopenwith.exe
)

add_executable(openwith_manifest_test test/manifest_test.cpp)
add_dependencies(openwith_manifest_test openwith_manifest_stage)
add_test(NAME openwith_manifest_test COMMAND openwith_manifest_test ${OPENWITH_MANIFEST_STAGING_DIR})

install(TARGETS shellopenwith RUNTIME DESTINATION .)
install(FILES plugin.json DESTINATION .)
