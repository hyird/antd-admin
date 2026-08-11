cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

# Auto-detect vcpkg via VCPKG_ROOT when no toolchain file is given explicitly.
# Must happen before the first project() call.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" _vcpkg_root)
    if(EXISTS "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file (auto-detected from VCPKG_ROOT)")
        message(STATUS "vcpkg toolchain auto-detected: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "VCPKG_ROOT is set but vcpkg.cmake not found under it: ${_vcpkg_root}")
    endif()
    unset(_vcpkg_root)
endif()

project(antd_admin_service VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "static")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/.env" "${CMAKE_BINARY_DIR}/.env" COPYONLY)
endif()

option(ANTD_ADMIN_BUILD_WEB "Build the frontend as part of the service build" ON)

# Ruvia exposes three targets (core/http/web); we consume ruvia::web.
set(RUVIA_BUILD_CORE ON CACHE BOOL "" FORCE)
set(RUVIA_BUILD_HTTP ON CACHE BOOL "" FORCE)
set(RUVIA_BUILD_WEB ON CACHE BOOL "" FORCE)
set(RUVIA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RUVIA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RUVIA_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(RUVIA_ENABLE_MARIADB ON CACHE BOOL "" FORCE)
set(RUVIA_ENABLE_JWT ON CACHE BOOL "" FORCE)
set(RUVIA_ENABLE_REDIS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    ruvia
    GIT_REPOSITORY https://github.com/hyird/Ruvia.git
    GIT_TAG 13dfb7f9cafeb45ea30b5a17b590b807747079da # main, 2026-08-11
)
FetchContent_MakeAvailable(ruvia)

if(ANTD_ADMIN_BUILD_WEB)
    if(WIN32)
        find_program(NPM_EXECUTABLE NAMES npm.cmd npm)
    else()
        find_program(NPM_EXECUTABLE NAMES npm)
    endif()

    if(NOT NPM_EXECUTABLE)
        message(FATAL_ERROR "npm is required to build the frontend")
    endif()

    if(WIN32 AND NOT NPM_EXECUTABLE MATCHES "\\.cmd$")
        if(EXISTS "${NPM_EXECUTABLE}.cmd")
            set(NPM_EXECUTABLE "${NPM_EXECUTABLE}.cmd")
        else()
            get_filename_component(NPM_EXECUTABLE_DIR "${NPM_EXECUTABLE}" DIRECTORY)
            if(EXISTS "${NPM_EXECUTABLE_DIR}/npm.cmd")
                set(NPM_EXECUTABLE "${NPM_EXECUTABLE_DIR}/npm.cmd")
            endif()
        endif()
    endif()

    message(STATUS "Using npm: ${NPM_EXECUTABLE}")

    add_custom_target(antd_admin_web ALL
        COMMAND "${CMAKE_COMMAND}" -E env
            NO_COLOR=1
            FORCE_COLOR=0
            npm_config_color=false
            "${NPM_EXECUTABLE}" run build -- --clearScreen false --logLevel warn
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building frontend"
        VERBATIM
    )
endif()

# Backend is header-only; server.cpp is the single translation unit.
add_executable(antd_admin_service service/server.cpp)

target_include_directories(antd_admin_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(antd_admin_service
    PRIVATE
        ruvia::web
)

if(ANTD_ADMIN_BUILD_WEB)
    add_dependencies(antd_admin_service antd_admin_web)
endif()

if(MSVC)
    target_compile_options(antd_admin_service PRIVATE /W4 /permissive- /Zc:preprocessor /utf-8 /wd4819)
else()
    target_compile_options(antd_admin_service PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(ANTD_ADMIN_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}")

set_target_properties(antd_admin_service PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${ANTD_ADMIN_RUNTIME_OUTPUT_DIR}"
    OUTPUT_NAME "server"
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUT_CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${OUTPUT_CONFIG}" OUTPUT_CONFIG_UPPER)
        set_target_properties(antd_admin_service PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG_UPPER}" "${ANTD_ADMIN_RUNTIME_OUTPUT_DIR}"
        )
    endforeach()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env")
    add_custom_command(TARGET antd_admin_service POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/.env"
            "$<TARGET_FILE_DIR:antd_admin_service>/.env"
        COMMENT "Copying .env to runtime directory"
        VERBATIM
    )
endif()

install(TARGETS antd_admin_service RUNTIME DESTINATION bin)
