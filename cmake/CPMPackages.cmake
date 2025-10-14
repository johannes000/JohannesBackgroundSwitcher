

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.15.1
  OPTIONS "SPDLOG_BUILD_SHARED OFF" "SPDLOG_BUILD_STATIC ON"
)

CPMAddPackage(
  NAME glew
  GITHUB_REPOSITORY nigels-com/glew
  GIT_TAG glew-2.2.0
)

CPMAddPackage(
  NAME cereal
  GITHUB_REPOSITORY USCiLab/cereal
  GIT_TAG v1.3.2
  DOWNLOAD_ONLY ON
)

CPMAddPackage(
    NAME nativefiledialog-extended
    GITHUB_REPOSITORY btzy/nativefiledialog-extended
    GIT_TAG v1.2.1
    OPTIONS
        "NFD_BUILD_TESTS OFF" 
        "NFD_PORTAL OFF" 
)