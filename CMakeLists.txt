cmake_minimum_required(VERSION 3.31.0)
project(Background_Switcher VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)

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


set(BUILD_PATH ${PROJECT_SOURCE_DIR}/build/bin/Debug)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_PATH})
set(LIBRARY_OUTPUT_PATH ${BUILD_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# Alle .cpp- und .hpp-Dateien rekursiv suchen
file(GLOB_RECURSE CORE_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
)


# Executable erstellen
add_executable(${PROJECT_NAME} ${PLATFORM_SOURCES} ${CORE_SOURCES})

# Include-Verzeichnisse
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

# Link-Bibliotheken
target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    opengl32
    SDL3::SDL3
    spdlog::spdlog
)
# target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)

# # Assets kopieren
# set(SOURCE_ASSETS_DIR ${CMAKE_SOURCE_DIR})
# set(DEST_ASSETS_DIR ${EXECUTABLE_OUTPUT_PATH})
# add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -E env cmd /c ${CMAKE_SOURCE_DIR}/copy_assets.bat ${SOURCE_ASSETS_DIR} ${DEST_ASSETS_DIR}
#     COMMENT "Copying assets folder"
# )


# Debug/Release-Konfiguration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

if(SDL_ADDED)
  if(WIN32 AND SDL_SHARED)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_BINARY_DIR}/SDL3.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
      COMMENT "Copying SDL3.dll to output directory"
    )
  endif()
endif()