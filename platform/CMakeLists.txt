add_library(Platform
    Platform.hpp 
    Platform_Win32.cpp
)

# if(WIN32)
#     target_sources(Platform )
# elseif(APPLE)
#     target_sources(Platform src/Platform_Mac.mm)
# elseif(UNIX AND NOT APPLE)
#     target_sources(Platform src/Platform_Linux.cpp)
# endif()

target_link_libraries( 
    Platform
    # $<$<PLATFORM_ID:Windows>:user32>
    $<$<PLATFORM_ID:Windows>:ole32>
    spdlog::spdlog
)

include_directories(Platform INTERFACE
    ${CPM_PACKAGE_spdlog_SOURCE_DIR}/include
)