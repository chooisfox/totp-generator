set(CURRENT_LIBRARY_NAME cppotp)

FetchContent_Declare(
    ${CURRENT_LIBRARY_NAME}
    GIT_REPOSITORY https://github.com/RavuAlHemio/cpptotp.git
    GIT_TAG        master
)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_MakeAvailable(${CURRENT_LIBRARY_NAME})

target_include_directories(${CURRENT_LIBRARY_NAME} PUBLIC
    "${${CURRENT_LIBRARY_NAME}_SOURCE_DIR}/src"
)

list(APPEND PROJECT_LIBRARIES_LIST cppotp)
