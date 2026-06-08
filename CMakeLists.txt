cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
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
    GIT_TAG d15989a43c0dbe6e7e700e3b3d364983c0c707d9 # v0.0.1
)
FetchContent_MakeAvailable(cyra)

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
