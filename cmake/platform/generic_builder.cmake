include(FetchContent)
include(ExternalProject)

include(cmake/utils/list_all_subdirectories.cmake)

message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER_ID}")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}")
endif()

# [SOURCE DIRECTORIES]
set(PROJECT_MAIN_SRC_DIR   "${CMAKE_SOURCE_DIR}/src")

file(GLOB PROJECT_MAIN_SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_MAIN_SRC_DIR}/*.hpp"
    "${PROJECT_MAIN_SRC_DIR}/*.cpp"
)

list(APPEND PROJECT_INCLUDE_DIRS ${PROJECT_MAIN_SRC_DIR})

# [INCLUDE DIRECTORIES]
set(PROJECT_INCLUDE_DIRS)

# [LIBRARIES LIST]
set(PROJECT_LIBRARIES_LIST)

# [DIRECTORIES LIST]
set(PROJECT_DIRECTORIES_LIST)

# [LIBRARIES]
include(cmake/libraries/curl.cmake)
include(cmake/libraries/fmt.cmake)
include(cmake/libraries/spdlog.cmake)
include(cmake/libraries/cxxopts.cmake)
include(cmake/libraries/tomlplusplus.cmake)
include(cmake/libraries/totp.cmake)
include(cmake/libraries/common.cmake)
include(cmake/libraries/utils.cmake)
include(cmake/libraries/app.cmake)

# LINUX does not exclude MACOS, so we need to check it first
if(MACOS)
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
elseif(LINUX)
    include(cmake/platform/linux_builder.cmake)
elseif(WIN32)
    include(cmake/platform/windows_builder.cmake)
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

# [TARGET LNKING]
target_include_directories(${PROJECT_NAME} PRIVATE   ${PROJECT_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE   ${PROJECT_DIRECTORIES_LIST})
target_link_directories(${PROJECT_NAME}    PRIVATE   ${PROJECT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}      PRIVATE   ${PROJECT_LIBRARIES_LIST})

if(CMAKE_SYSTEM_TILING)
    target_compile_definitions(${CURRENT_LIBRARY_NAME} PUBLIC
        SYSTEM_IS_TILING="${CMAKE_SYSTEM_TILING}")
endif()

# [SOURCE GROUPS]
source_group("Main" FILES ${PROJECT_MAIN_SRC_FILES})

include(cmake/utils/postbuild_scripts.cmake)
