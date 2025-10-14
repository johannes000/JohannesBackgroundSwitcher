CPMAddPackage(
  NAME SDL
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG release-3.2.8
  OPTIONS 
    "SDL_SHARED OFF" 
    "SDL_STATIC ON"
    "SDL_TESTS OFF"
)

CPMAddPackage(
  NAME SDL_ttf
  GITHUB_REPOSITORY libsdl-org/SDL_ttf
  GIT_TAG release-3.2.2
  OPTIONS 
    "SDLTTF_VENDORED ON"
    "SDLTTF_HARFBUZZ OFF"
)

CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.91.9b-docking
)

add_library(Renderer STATIC
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_demo.cpp 
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_draw.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_tables.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(Renderer PUBLIC
  ${CPM_PACKAGE_imgui_SOURCE_DIR}
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends
  ${CPM_PACKAGE_SDL_SOURCE_DIR}/include
  ${CPM_PACKAGE_SDL_ttf_SOURCE_DIR}/include
)

target_link_libraries(Renderer PUBLIC 
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)

find_package(OpenGL REQUIRED)
target_link_libraries(Renderer PUBLIC OpenGL::GL)

target_compile_definitions(Renderer PUBLIC 
  IMGUI_IMPL_OPENGL_LOADER_GL3W
  IMGUI_DISABLE_OBSOLETE_FUNCTIONS
)
if(WIN32)
    target_link_libraries(Renderer PRIVATE opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_VIDEO_LIBRARY CoreVideo)
    target_link_libraries(Renderer PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${CORE_VIDEO_LIBRARY})
elseif(UNIX)
    target_link_libraries(Renderer PRIVATE X11)
endif()