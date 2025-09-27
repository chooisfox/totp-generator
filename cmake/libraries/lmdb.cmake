set(CURRENT_LIBRARY_NAME lmdb)

FetchContent_Declare(
    ${CURRENT_LIBRARY_NAME}
    GIT_REPOSITORY https://github.com/LMDB/lmdb.git
    GIT_TAG        LMDB_0.9.31
)
FetchContent_MakeAvailable(${CURRENT_LIBRARY_NAME})

add_library(${CURRENT_LIBRARY_NAME} STATIC
  ${lmdb_SOURCE_DIR}/libraries/liblmdb/mdb.c
  ${lmdb_SOURCE_DIR}/libraries/liblmdb/midl.c
)

# We tell any target that uses 'lmdb' where to find its public header file (lmdb.h).
target_include_directories(${CURRENT_LIBRARY_NAME} PUBLIC
  ${lmdb_SOURCE_DIR}/libraries/liblmdb
)
list(APPEND PROJECT_LIBRARIES_LIST ${CURRENT_LIBRARY_NAME})
