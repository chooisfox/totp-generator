set(CURRENT_LIBRARY_NAME lmdbxx)

FetchContent_Declare(
    ${CURRENT_LIBRARY_NAME}
    GIT_REPOSITORY https://github.com/hoytech/lmdbxx.git
    GIT_TAG        1.0.0
)
FetchContent_MakeAvailable(${CURRENT_LIBRARY_NAME})

add_library(lmdbxx-wrapper INTERFACE)
target_include_directories(lmdbxx-wrapper INTERFACE
    ${${CURRENT_LIBRARY_NAME}_SOURCE_DIR}
)

list(APPEND PROJECT_LIBRARIES_LIST lmdbxx-wrapper)
