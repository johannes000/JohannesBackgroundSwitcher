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

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Abhängigkeiten
include(cmake/CPM.cmake)
include(cmake/warnings.cmake)

CPMAddPackage(
  NAME SDL
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG release-3.2.8
  OPTIONS "SDL_SHARED ON" "SDL_STATIC OFF"
)

CPMAddPackage(
  NAME FreeImage
  GITHUB_REPOSITORY danoli3/FreeImage
  GIT_TAG 3.19.10
  OPTIONS
    "FREEIMAGE_BUILD_ZLIB TRUE"
    "FREEIMAGE_BUILD_JPEG TRUE"
    "FREEIMAGE_BUILD_PNG TRUE"
    "FREEIMAGE_BUILD_STATIC TRUE"
)

CPMAddPackage(
    NAME nativefiledialog-extended
    GITHUB_REPOSITORY btzy/nativefiledialog-extended
    GIT_TAG v1.2.1
    OPTIONS
        "NFD_BUILD_TESTS OFF" 
        "NFD_PORTAL OFF" 
)

CPMAddPackage(
  NAME nlohmann-json
  VERSION 3.12.0 
  GITHUB_REPOSITORY nlohmann/json
  OPTIONS
    "JSON_BuildTests OFF" 
    "JSON_MultipleHeaders OFF"
    "JSON_Install OFF" 
)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.15.1
  OPTIONS "SPDLOG_BUILD_SHARED OFF" "SPDLOG_BUILD_STATIC ON"
)

CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.91.9b-docking
)

CPMAddPackage(
  NAME glew
  GITHUB_REPOSITORY nigels-com/glew
  GIT_TAG glew-2.2.0
)

include(FetchContent)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp # optionally comment this out
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

include_directories(imgui INTERFACE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${SDL_SOURCE_DIR}/include
)

target_compile_definitions(imgui PUBLIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS) # optional imgui setting
set_target_properties(imgui PROPERTIES FOLDER third-party) # optoinal IDE dir

# Für Multi-Config Generatoren (Visual Studio, Xcode)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
# set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# CPM Konfiguration für Debug/Release builds
set(CPM_USE_LOCAL_PACKAGES ON)  # Bessere Wiederverwendung zwischen Builds

# Alle .cpp- und .hpp-Dateien rekursiv suchen
file(GLOB_RECURSE CORE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/app_icon.rc"
)


# Executable erstellen
add_executable(${PROJECT_NAME} ${PLATFORM_SOURCES} ${CORE_SOURCES})

# Include-Verzeichnisse
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CPM_PACKAGE_FreeImage_SOURCE_DIR}/Source
    ${CPM_PACKAGE_nativefiledialog-extended_SOURCE_DIR}/src/include
    ${CPM_PACKAGE_nlohmann-json_SOURCE_DIR}/include
    ${CPM_PACKAGE_spdlog_SOURCE_DIR}/include
)

# Link-Bibliotheken
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<PLATFORM_ID:Windows>:user32>
    imgui
    opengl32
    SDL3::SDL3
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    FreeImage
    nfd # nativefiledialog-extended
)
target_compile_definitions(${PROJECT_NAME} PRIVATE FREEIMAGE_LIB)
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

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
  )
endif()

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

