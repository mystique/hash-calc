cmake_minimum_required(VERSION 3.20)

project(HashCalc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(cryptopp CONFIG REQUIRED)

# Auto-discover sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Win32 GUI application (no console window)
add_executable(${PROJECT_NAME} WIN32
    ${APP_SOURCES}
    res/resource.rc
    res/app.manifest
)

# Include directories (res before cryptopp to avoid resource.h conflict)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/win32xx
    ${CMAKE_SOURCE_DIR}/res
    ${CMAKE_SOURCE_DIR}/src
)

# Link required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    comctl32
    uxtheme
    cryptopp::cryptopp
)

# Enable Unicode
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
)
