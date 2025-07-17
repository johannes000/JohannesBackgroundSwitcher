
# include(CTest)
enable_testing()
include(GoogleTest)

# Executable erstellen
file(GLOB TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/test/*.cpp"
)
add_executable(Test ${TEST_SOURCES})

target_include_directories(Test PRIVATE
${PROJECT_SOURCE_DIR}/src
${CPM_PACKAGE_FreeImage_SOURCE_DIR}/Source
${CPM_PACKAGE_nativefiledialog-extended_SOURCE_DIR}/src/include
${CPM_PACKAGE_nlohmann-json_SOURCE_DIR}/include
${CPM_PACKAGE_spdlog_SOURCE_DIR}/include
)

target_link_libraries(Test PRIVATE
gtest gtest_main gmock
$<$<PLATFORM_ID:Windows>:user32>
# imgui
# opengl32
# SDL3::SDL3
# spdlog::spdlog
# nlohmann_json::nlohmann_json
# FreeImage
# nfd # nativefiledialog-extended
)
target_compile_definitions(Test PRIVATE FREEIMAGE_LIB)

gtest_discover_tests(Test)
# add_test( Test  ${CMAKE_CURRENT_BINARY_DIR}/${filename} )