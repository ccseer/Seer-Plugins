cmake_minimum_required(VERSION 3.21)
project(sha256_property LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(sha256_core STATIC src/sha256.cpp)
target_include_directories(sha256_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(sha256_core PUBLIC bcrypt)

add_executable(sha256_property src/main.cpp)
target_link_libraries(sha256_property PRIVATE sha256_core shell32)

add_executable(sha256_property_test test/sha256_test.cpp)
target_link_libraries(sha256_property_test PRIVATE sha256_core)
add_test(NAME sha256_property_test COMMAND sha256_property_test)

set(SHA256_PROPERTY_MANIFEST_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/manifest-stage)
add_custom_command(
    OUTPUT
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/plugin.json
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/sha256_property.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/plugin.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sha256_property>
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/sha256_property.exe
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        sha256_property
    VERBATIM
)
add_custom_target(sha256_property_manifest_stage
    DEPENDS
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/plugin.json
        ${SHA256_PROPERTY_MANIFEST_STAGING_DIR}/sha256_property.exe
)

add_executable(sha256_property_manifest_test test/manifest_test.cpp)
add_dependencies(sha256_property_manifest_test sha256_property_manifest_stage)
add_test(NAME sha256_property_manifest_test
         COMMAND sha256_property_manifest_test ${SHA256_PROPERTY_MANIFEST_STAGING_DIR})

install(TARGETS sha256_property RUNTIME DESTINATION .)
install(FILES plugin.json DESTINATION .)
