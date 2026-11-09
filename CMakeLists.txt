cmake_minimum_required(VERSION 3.15)
set(APP_VERSION "undefined" CACHE STRING "Application version string")
if(APP_VERSION STREQUAL "undefined")
    project(frameIntervalCheck LANGUAGES CXX)
else()
    project(frameIntervalCheck VERSION ${APP_VERSION} LANGUAGES CXX)
endif()


add_compile_definitions(CMAKE_BUILD)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WINDOWS_RUB_INCLUDE "" CACHE PATH "Path to windowsRub include directory")
if(NOT WINDOWS_RUB_INCLUDE)
    message(FATAL_ERROR "windowsRub not configured. Set -DWINDOWS_RUB_INCLUDE=...")
endif()

set(CXXOPTS_ROOT "" CACHE PATH "Path to cxxopts root directory")
if(CXXOPTS_ROOT)
    message(STATUS "set cxxopts with CXXOPTS_ROOT = '${CXXOPTS_ROOT} .'")
    set(CXXOPTS_INCLUDE_DIRS ${CXXOPTS_ROOT}/include)
else()
    message(STATUS "CXXOPTS_ROOT not set, automatically finding cxxopts.")
    find_package(cxxopts CONFIG QUIET)
    if(cxxopts_FOUND)
        get_target_property(CXXOPTS_INCLUDE_DIRS cxxopts::cxxopts INTERFACE_INCLUDE_DIRECTORIES)
    else()
        message(FATAL_ERROR "cxxopts not configured. Set -DCXXOPTS_ROOT=...")
    endif()
endif()

set(FFMPEG_ROOT "" CACHE PATH "Path to ffmpeg root directory")
if(FFMPEG_ROOT)
    message(STATUS "FFMPEG_ROOT set, finding libraries with FFMPEG_ROOT = '${FFMPEG_ROOT}' .")
    set(FFMPEG_INCLUDE_DIRS "${FFMPEG_ROOT}/include")
    find_library(AVFORMAT_LIBRARY
        NAMES avformat
        PATHS ${FFMPEG_ROOT}/lib
    )
    find_library(AVCODEC_LIBRARY
        NAMES avcodec
        PATHS ${FFMPEG_ROOT}/lib
    )
    find_library(AVUTIL_LIBRARY
        NAMES avutil
        PATHS ${FFMPEG_ROOT}/lib
    )
else()
    message(STATUS "FFMPEG_ROOT not set, automatically finding FFmpeg.")
    find_path(FFMPEG_INCLUDE_DIRS
        NAMES libavformat/avformat.h
        PATHS $ENV{INCLUDE}
    )
    find_library(AVFORMAT_LIBRARY
        NAMES avformat
        PATHS $ENV{LIB}
    )
    find_library(AVCODEC_LIBRARY
        NAMES avcodec
        PATHS $ENV{LIB}
    )
    find_library(AVUTIL_LIBRARY
        NAMES avutil
        PATHS $ENV{LIB}
    )
endif()
if(NOT FFMPEG_INCLUDE_DIRS OR NOT AVFORMAT_LIBRARY OR NOT AVCODEC_LIBRARY OR NOT AVUTIL_LIBRARY)
    message(FATAL_ERROR "FFmpeg not configured, please confirm your installation.")
endif()

add_executable(frameIntervalCheck src/checker.cpp
    src/files.cpp
    src/ffutils.cpp
    src/calculate.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(frameIntervalCheck PROPERTIES
        INSTALL_RPATH "$ORIGIN/lib"
    )
endif()

target_include_directories(frameIntervalCheck PRIVATE
    ${WINDOWS_RUB_INCLUDE}
    ${CXXOPTS_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)
target_link_libraries(frameIntervalCheck PRIVATE
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVUTIL_LIBRARY}
)

if(DEFINED ENV{RUNNER_TEMP})
    set(COMPILER_TMP "$ENV{RUNNER_TEMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
elseif(DEFINED ENV{TMP})
    set(COMPILER_TMP "$ENV{TMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
elseif(DEFINED ENV{TEMP})
    set(COMPILER_TMP "$ENV{TEMP}/compiler_${PROJECT_NAME}" CACHE PATH "Path to compiler temp directory")
endif()
if(COMPILER_TMP)
    configure_file(
        src/compiler/version.h.in
        ${COMPILER_TMP}/version.h
        @ONLY
    )
    target_include_directories(frameIntervalCheck PRIVATE
        ${COMPILER_TMP}
    )
endif()

if(MSVC)
    target_compile_options(frameIntervalCheck PRIVATE /utf-8 /EHsc)
    set_target_properties(frameIntervalCheck PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()