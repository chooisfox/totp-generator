include(cmake/utils/get_windows_version.cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "Build as debug" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(
            /O2
            /Ob2
            /Oi
            /Ot
            /GL
            /Gy
            /Gw
            /GS-
            /Gy
            /W4
            /permissive-
            /volatile:iso
            /EHsc
            /GR
            /Z7
            /pdb:none
            /Zc:forScope
            /Zc:inline
            /Zc:rvalueCast
            /Zc:strictStrings
            /Zc:throwingNew
            /Zc:wchar_t
        )
    elseif(MINGW)
        add_compile_options(
            -O2
            -fomit-frame-pointer
            -funroll-loops
            -Wall
            -Wextra
            -Werror
            -fno-strict-aliasing
            -fno-common
            -D__USE_MINGW_ANSI_STDIO=1
        )
    elseif(CLANG)
        add_compile_options(
            -O2
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -fno-strict-aliasing
            -fno-common
        )
    endif()
else()
    if(MSVC)
        add_compile_options(
            /W4
            /WX-
            /MDd
        )
    elseif(MINGW)
        add_compile_options(
            -Wall
            -g
            -DDEBUG
        )
    elseif(CLANG)
        add_compile_options(
            -Wall
            -g
            -DDEBUG
        )
    endif()
endif()

# [EXECUTABLE]
add_executable(${PROJECT_NAME} ${PROJECT_MAIN_SRC_FILES})

# [INSTALLATION]
include(InstallRequiredSystemLibraries)

install(DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/
        DESTINATION bin
        FILES_MATCHING
        PATTERN "*.dll"
        PATTERN "*.exe"
)

set(CPACK_GENERATOR "NSIS")

set(CPACK_NSIS_COMPRESSION_TYPE "lzma")
set(CPACK_NSIS_COMPRESSION_LEVEL 9)

set(CPACK_PACKAGE_NAME                "${PROJECT_APPLICATION_NAME}")
set(CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY   "${PROJECT_ORGANIZATION_NAME}\\${PROJECT_APPLICATION_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${Project_DESCRIPTION}")
set(CPACK_RPM_PACKAGE_LICENSE         "MIT")
set(CPACK_PACKAGE_VENDOR              "${PROJECT_ORGANIZATION_NAME}")
set(CPACK_PACKAGE_CONTACT             "${PROJECT_DEVELOPER_EMAIL}")
set(CPACK_PACKAGE_EXECUTABLES         "${PROJECT_NAME}" "${PROJECT_NAME}")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_APPLICATION_NAME} Installer")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\${PROJECT_NAME}.exe")
#set(CPACK_NSIS_ICON "${CMAKE_SOURCE_DIR}/installer_icon.ico")
set(CPACK_NSIS_CONTACT "${PROJECT_DEVELOPER_EMAIL}")
set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_ORGANIZATION_WEBSITE}")
set(CPACK_NSIS_FILE_ASSOCIATIONS ".txt=${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe")

include(CPack)
