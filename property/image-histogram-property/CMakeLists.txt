cmake_minimum_required(VERSION 3.21)
project(image_histogram LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Test)

get_filename_component(QT_BIN_DIR "${Qt6Core_DIR}/../../../bin" ABSOLUTE)

include(CTest)
enable_testing()

add_library(image_histogram_core STATIC
    src/histogram.cpp
    src/histogramrenderer.cpp
    src/histogramhelper.cpp
)
target_include_directories(image_histogram_core PUBLIC src)
target_link_libraries(image_histogram_core PUBLIC Qt6::Core Qt6::Gui)

add_executable(image_histogram
    src/main.cpp
)
target_link_libraries(image_histogram PRIVATE image_histogram_core)

add_executable(image_histogram_test
    test/histogram_test.cpp
)
target_link_libraries(image_histogram_test PRIVATE image_histogram_core Qt6::Test)
add_test(NAME image_histogram_test COMMAND image_histogram_test)
set_tests_properties(image_histogram_test PROPERTIES
    ENVIRONMENT "PATH=${QT_BIN_DIR};$ENV{PATH}"
)

add_executable(image_histogram_renderer_test
    test/histogramrenderer_test.cpp
)
target_link_libraries(image_histogram_renderer_test PRIVATE image_histogram_core Qt6::Test)
add_test(NAME image_histogram_renderer_test COMMAND image_histogram_renderer_test)
set_tests_properties(image_histogram_renderer_test PROPERTIES
    ENVIRONMENT "PATH=${QT_BIN_DIR};$ENV{PATH}"
)

add_executable(image_histogram_output_test
    test/output_contract_test.cpp
)
target_link_libraries(image_histogram_output_test PRIVATE image_histogram_core Qt6::Test)
add_test(NAME image_histogram_output_test COMMAND image_histogram_output_test)
set_tests_properties(image_histogram_output_test PROPERTIES
    ENVIRONMENT "PATH=${QT_BIN_DIR};$ENV{PATH}"
)

add_executable(image_histogram_manifest_test
    test/manifest_test.cpp
)
target_include_directories(image_histogram_manifest_test PRIVATE src)
target_link_libraries(image_histogram_manifest_test PRIVATE Qt6::Core Qt6::Gui)

set(IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR ${CMAKE_CURRENT_BINARY_DIR}/manifest-stage)
add_custom_command(
    OUTPUT
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/plugin.json
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/image_histogram.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/plugin.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:image_histogram>
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/image_histogram.exe
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.json
        image_histogram
    VERBATIM
)
add_custom_target(image_histogram_manifest_stage
    DEPENDS
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/plugin.json
        ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR}/image_histogram.exe
)
add_dependencies(image_histogram_manifest_test image_histogram_manifest_stage)
add_test(NAME image_histogram_manifest_test
         COMMAND image_histogram_manifest_test ${IMAGE_HISTOGRAM_MANIFEST_STAGING_DIR})
set_tests_properties(image_histogram_manifest_test PROPERTIES
    ENVIRONMENT "PATH=${QT_BIN_DIR};$ENV{PATH}"
)

install(TARGETS image_histogram RUNTIME DESTINATION .)
install(FILES plugin.json DESTINATION .)
