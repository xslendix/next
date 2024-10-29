cmake_minimum_required(VERSION 3.19...3.24)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

project(ByteRacer
    VERSION 0.1.0
    LANGUAGES C CXX)

# Detect if we are using Emscripten
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set(EMSCRIPTEN ON)
else()
    set(EMSCRIPTEN OFF)
endif()

# Set RAYLIB_TARGET_PLATFORM variable based on the target
if(EMSCRIPTEN)
    set(RAYLIB_TARGET_PLATFORM "Web")
else()
    set(RAYLIB_TARGET_PLATFORM "Desktop")
endif()

# ---- Add dependencies via CPM ----
set(CPM_DOWNLOAD_VERSION 0.40.2)
if(CPM_SOURCE_CACHE)
    get_filename_component(CPM_SOURCE_CACHE ${CPM_SOURCE_CACHE} ABSOLUTE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Set raylib options before including it
set(PLATFORM ${RAYLIB_TARGET_PLATFORM} CACHE STRING "Platform for raylib" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add raylib (3rd-party)
CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG master
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.11.2  # Set the specific version you want, or omit to get the latest
)

# Compiler options for raylib
if(MSVC)
    target_compile_options(raylib PRIVATE /wd4100 /wd4204)
else()
    target_compile_options(raylib PRIVATE -Wno-error=implicit-function-declaration -Wno-unused-result)
endif()

# Add your project sources
add_subdirectory(src)

# If using MSVC and ASAN, set debugger environment
#if(MSVC)
#    # Collect all targets into `all_targets` list manually or by using installable targets approach.
#    set(all_targets "")
#    get_property(all_target_names GLOBAL PROPERTY TARGETS)
#    foreach(target ${all_target_names})
#        get_target_property(is_imported ${target} IMPORTED)
#        if(NOT is_imported)
#            list(APPEND all_targets ${target})
#        endif()
#    endforeach()
#
#    message("all_targets=${all_targets}")
#
#    # Set environment for each target
#    foreach(target ${all_targets})
#        set_target_properties(${target} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
#    endforeach()
#endif()
