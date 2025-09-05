cmake_minimum_required(VERSION 3.31.0)
project(Background_Switcher VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3 -O0)  # Für GCC/Clang
    if(MSVC)
        add_compile_options(/Zi /Od)  # Für Visual Studio
    endif()
endif()

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Abhängigkeiten
include(cmake/warnings.cmake)

include(FetchContent)
include(cmake/CPM.cmake)
include(cmake/CPMPackages.cmake)
include(cmake/imgui.cmake)
include(cmake/freeimage.cmake)


set(LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/extern/lib)

# Installationsregel für DLLs


# Für Multi-Config Generatoren (Visual Studio, Xcode)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
# set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(CPM_USE_LOCAL_PACKAGES ON)  # Bessere Wiederverwendung zwischen Builds

file(GLOB_RECURSE CORE_SOURCES
    src/main.cpp
    src/app_icon.rc

    src/App.cpp src/App.hpp
    src/GUI.cpp src/GUI.hpp
)

add_subdirectory(platform)

add_executable(${PROJECT_NAME} ${PLATFORM_SOURCES} ${CORE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/platform
    ${PROJECT_SOURCE_DIR}/extern/include

    ${CPM_PACKAGE_cereal_SOURCE_DIR}/include
    ${CPM_PACKAGE_FreeImage_SOURCE_DIR}/Source
    ${CPM_PACKAGE_imgui_SOURCE_DIR}
    ${CPM_PACKAGE_nativefiledialog-extended_SOURCE_DIR}/src/include
    ${CPM_PACKAGE_SDL_SOURCE_DIR}/include
    ${CPM_PACKAGE_spdlog_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    opengl32
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    spdlog::spdlog
    FreeImage
    Platform
    nfd # nativefiledialog-extended
)
target_compile_definitions(${PROJECT_NAME} PRIVATE FREEIMAGE_LIB)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${LIB_OUTPUT_DIR}
    LIBRARY DESTINATION ${LIB_OUTPUT_DIR}
)
# target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)

# # Assets kopieren
# set(SOURCE_ASSETS_DIR ${CMAKE_SOURCE_DIR})
# set(DEST_ASSETS_DIR ${EXECUTABLE_OUTPUT_PATH})
# add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -E env cmd /c ${CMAKE_SOURCE_DIR}/copy_assets.bat ${SOURCE_ASSETS_DIR} ${DEST_ASSETS_DIR}
#     COMMENT "Copying assets folder"
# )


if(SDL_ADDED)
  if(WIN32 AND SDL_SHARED)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SDL_BINARY_DIR}/SDL3.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying SDL3.dll to output directory"
  )
  endif()
endif()

# if(CMAKE_BUILD_TYPE STREQUAL "Release")
#   install(TARGETS ${PROJECT_NAME}
#     RUNTIME DESTINATION bin
#     BUNDLE DESTINATION bin
#   )
# endif()

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
  COMMENT "Copying assets to output directory"
  VERBATIM
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:WINDOWS")
    elseif(MINGW)
        target_link_options(${PROJECT_NAME} PRIVATE "-mwindows")
    endif()
endif()

