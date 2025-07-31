set(EXTERN_DIR ${CMAKE_SOURCE_DIR}/extern)
# FreeImage
find_path(FREEIMAGE_INCLUDE_DIR FreeImage.h PATHS ${EXTERN_DIR}/include REQUIRED)
find_library(FREEIMAGE_LIBRARY FreeImage PATHS ${EXTERN_DIR}/lib REQUIRED)

# find_path(FREEIMAGE_INCLUDE_DIR FreeImage.h
#     PATHS ${CMAKE_SOURCE_DIR}/extern/include
#     REQUIRED
# )

# find_library(FREEIMAGE_LIBRARY
#     NAMES FreeImage freeimage
#     PATHS ${CMAKE_SOURCE_DIR}/extern/lib
#     REQUIRED
# )