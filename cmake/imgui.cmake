CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.91.9b-docking
)
add_library(imgui STATIC
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_demo.cpp 
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_draw.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/imgui_tables.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

include_directories(imgui INTERFACE
  ${CPM_PACKAGE_imgui_SOURCE_DIR}
  ${CPM_PACKAGE_imgui_SOURCE_DIR}/backends
  ${CPM_PACKAGE_SDL_SOURCE_DIR}/include
  )