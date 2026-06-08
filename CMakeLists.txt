cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_ROOT_CANDIDATES
        "$ENV{VCPKG_ROOT}"
        "$ENV{VCPKG_INSTALLATION_ROOT}"
        "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg"
        "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg"
        "$ENV{HOME}/vcpkg"
        "$ENV{HOME}/Dev/vcpkg"
        "$ENV{HOME}/Workspace/vcpkg"
        "/opt/vcpkg"
        "/opt/homebrew/opt/vcpkg"
        "/opt/homebrew/share/vcpkg"
        "/usr/local/opt/vcpkg"
        "/usr/local/share/vcpkg"
        "/usr/local/vcpkg"
        "D:/Dev/vcpkg"
        "D:/vcpkg"
        "C:/vcpkg"
    )

    find_program(VCPKG_EXECUTABLE NAMES vcpkg vcpkg.exe)
    if(VCPKG_EXECUTABLE)
        get_filename_component(VCPKG_EXECUTABLE_REAL "${VCPKG_EXECUTABLE}" REALPATH)
        get_filename_component(VCPKG_EXECUTABLE_DIR "${VCPKG_EXECUTABLE_REAL}" DIRECTORY)
        get_filename_component(VCPKG_EXECUTABLE_PARENT "${VCPKG_EXECUTABLE_DIR}" DIRECTORY)
        list(APPEND VCPKG_ROOT_CANDIDATES
            "${VCPKG_EXECUTABLE_DIR}"
            "${VCPKG_EXECUTABLE_PARENT}"
            "${VCPKG_EXECUTABLE_PARENT}/share/vcpkg"
        )
    endif()

    foreach(VCPKG_ROOT_CANDIDATE IN LISTS VCPKG_ROOT_CANDIDATES)
        if(NOT VCPKG_ROOT_CANDIDATE)
            continue()
        endif()

        if(EXISTS "${VCPKG_ROOT_CANDIDATE}/scripts/buildsystems/vcpkg.cmake")
            set(VCPKG_ROOT "${VCPKG_ROOT_CANDIDATE}" CACHE PATH "vcpkg root")
            set(CMAKE_TOOLCHAIN_FILE
                "${VCPKG_ROOT_CANDIDATE}/scripts/buildsystems/vcpkg.cmake"
                CACHE FILEPATH "vcpkg toolchain file"
            )
            message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
            break()
        endif()
    endforeach()
endif()

if(NOT VCPKG_TARGET_TRIPLET AND WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
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

set(CYRA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CYRA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CYRA_ENABLE_MEMORY_STATS OFF CACHE BOOL "" FORCE)
set(CYRA_ENABLE_STD_REGEX_RULES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    cyra
    GIT_REPOSITORY https://github.com/hyird/cyra.git
    GIT_TAG v0.0.1
)
FetchContent_MakeAvailable(cyra)

if(ANTD_ADMIN_BUILD_WEB)
    find_program(NPM_EXECUTABLE NAMES npm npm.cmd)

    if(NOT NPM_EXECUTABLE)
        message(FATAL_ERROR "npm is required to build the frontend")
    endif()

    message(STATUS "Using npm: ${NPM_EXECUTABLE}")

    if(WIN32)
        add_custom_target(antd_admin_web ALL
            COMMAND cmd /c "chcp 65001 >nul 2>&1 && \"${NPM_EXECUTABLE}\" run build"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Building frontend"
        )
    else()
        add_custom_target(antd_admin_web ALL
            COMMAND "${NPM_EXECUTABLE}" run build
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Building frontend"
            VERBATIM
        )
    endif()
endif()

# 后端是 header-only：只有 service/server.cpp 一个翻译单元，业务模块通过 .h 内联实现。
add_executable(antd_admin_service service/server.cpp)

target_include_directories(antd_admin_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(antd_admin_service
    PRIVATE
        cyra::cyra
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
