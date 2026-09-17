cmake_minimum_required(VERSION 3.21)
project(explorer_properties LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
enable_testing()

add_library(shellverb_core STATIC src/shellverb.cpp)
target_include_directories(shellverb_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(shellverb_properties WIN32 src/main.cpp)
target_include_directories(shellverb_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common)

target_link_libraries(shellverb_properties PRIVATE shellverb_core Shell32.lib)

add_executable(shellverb_properties_test test/main_test.cpp)
target_link_libraries(shellverb_properties_test PRIVATE shellverb_core Shell32.lib)
add_test(NAME shellverb_properties_test COMMAND shellverb_properties_test)

set(SHELLVERB_PACKAGE_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/package-stage)
add_custom_command(
    OUTPUT
        ${SHELLVERB_PACKAGE_STAGING_DIR}/plugin.json
        ${SHELLVERB_PACKAGE_STAGING_DIR}/shellverb_properties.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHELLVERB_PACKAGE_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${SHELLVERB_PACKAGE_STAGING_DIR}/plugin.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:shellverb_properties>
        ${SHELLVERB_PACKAGE_STAGING_DIR}/shellverb_properties.exe
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json shellverb_properties
    VERBATIM
)
add_custom_target(shellverb_manifest_stage
    DEPENDS
        ${SHELLVERB_PACKAGE_STAGING_DIR}/plugin.json
        ${SHELLVERB_PACKAGE_STAGING_DIR}/shellverb_properties.exe
)

add_executable(shellverb_manifest_test test/manifest_test.cpp)
add_dependencies(shellverb_manifest_test shellverb_manifest_stage)
add_test(NAME shellverb_manifest_test
    COMMAND shellverb_manifest_test ${SHELLVERB_PACKAGE_STAGING_DIR})
add_custom_target(shellverb_package
    COMMAND $<TARGET_FILE:shellverb_manifest_test> ${SHELLVERB_PACKAGE_STAGING_DIR}
    DEPENDS shellverb_manifest_test
    VERBATIM
)

install(TARGETS shellverb_properties RUNTIME DESTINATION .)
install(FILES plugin.json DESTINATION .)
install(CODE [=[
    set(_package_root "${CMAKE_INSTALL_PREFIX}")
    file(REAL_PATH "${_package_root}" _package_root_real)
    set(_manifest "${_package_root_real}/plugin.json")
    set(_helper "${_package_root_real}/shellverb_properties.exe")

    if(NOT EXISTS "${_manifest}" OR IS_DIRECTORY "${_manifest}")
        message(FATAL_ERROR "Explorer Properties package is missing plugin.json")
    endif()
    if(NOT EXISTS "${_helper}" OR IS_DIRECTORY "${_helper}")
        message(FATAL_ERROR "Explorer Properties package is missing shellverb_properties.exe")
    endif()

    file(REAL_PATH "${_manifest}" _manifest_real)
    file(RELATIVE_PATH _manifest_relative "${_package_root_real}" "${_manifest_real}")
    if(IS_ABSOLUTE "${_manifest_relative}" OR _manifest_relative MATCHES "^\\.\\.(/|\\\\|$)")
        message(FATAL_ERROR "plugin.json resolves outside the package root")
    endif()

    file(REAL_PATH "${_helper}" _helper_real)
    file(RELATIVE_PATH _helper_relative "${_package_root_real}" "${_helper_real}")
    if(IS_ABSOLUTE "${_helper_relative}" OR _helper_relative MATCHES "^\\.\\.(/|\\\\|$)")
        message(FATAL_ERROR "shellverb_properties.exe resolves outside the package root")
    endif()

    file(READ "${_manifest_real}" _manifest_json)
    string(JSON _id_check ERROR_VARIABLE _json_error GET "${_manifest_json}" id)
    if(_json_error AND NOT _json_error STREQUAL "NOTFOUND")
        message(FATAL_ERROR "invalid Explorer Properties plugin.json: ${_json_error}")
    endif()
    string(JSON _id GET "${_manifest_json}" id)
    string(JSON _backend GET "${_manifest_json}" backend)
    string(JSON _command GET "${_manifest_json}" command)
    string(JSON _extensions_length LENGTH "${_manifest_json}" extensions)
    string(JSON _capabilities_length LENGTH "${_manifest_json}" capabilities)
    string(JSON _extension GET "${_manifest_json}" extensions 0)
    string(JSON _capability GET "${_manifest_json}" capabilities 0)

    if(NOT _id STREQUAL "io.1218.seer.explorer-properties"
       OR NOT _backend STREQUAL "process"
       OR NOT _command STREQUAL "shellverb_properties.exe"
       OR NOT _extensions_length EQUAL 1
       OR NOT _extension STREQUAL "\${type_file}"
       OR NOT _capabilities_length EQUAL 1
       OR NOT _capability STREQUAL "control")
        message(FATAL_ERROR "Explorer Properties plugin.json does not match its fixed package contract")
    endif()
]=])
