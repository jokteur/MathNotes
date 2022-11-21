# Set C++ 17 compiler flags
if(MSVC)
    cmake_minimum_required(VERSION 3.21)
else()
    cmake_minimum_required(VERSION 3.17)
endif()

set(CMAKE_CXX_STANDARD 17)

# Set project name
project(MathNotes)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(INSTALL_DIR "0.1")

# ---- MicroTex ----
add_subdirectory(external/microtex)
include_directories(external/microtex)

# ---- Cairo ----
find_package(Cairo)
include_directories(${CAIRO_INCLUDE_DIRS})

# ---- Tempo app ----
# ImGui config
add_compile_definitions(IMGUI_USE_WCHAR32)
add_subdirectory(external/TempoApp)
include_directories(external/TempoApp/include)
include_directories(external/TempoApp/external/imgui/imgui)

# ---- Markdown (md4c) ----
add_compile_definitions(MD4C_USE_UTF8)
add_subdirectory(external/md4c)
include_directories(external/md4c/src)

# -----
# Fonts
# -----
include_directories(external/IconFontCppHeaders)

# -------
# DocTest
# -------
add_subdirectory(external/doctest)

# ---------------
# STB image write
# ---------------
include_directories(external/stb_image)

# Sources of program
include_directories(src)
file(GLOB source_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/core/*.cpp
    src/ab/*.cpp
    src/rich_text/*.cpp
    src/rich_text/chars/*.cpp
    src/rich_text/spans/*.cpp
    src/rich_text/blocks/*.cpp
    src/latex/*.cpp
    src/translations/*.cpp
    src/ui/*.cpp src/ui/*.h
)

# Making a library without the main, for the tests and the program
add_library(${PROJECT_NAME}_lib ${source_list})

# Main program executable
add_executable(${PROJECT_NAME} src/main.cpp ${APP_ICON_RESOURCE_WINDOWS})

set(LIB_LINK microtex-imgui ${CAIRO_LIBRARIES} md4c md4c-html)
target_link_libraries(${PROJECT_NAME}_lib ${LIB_LINK})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib ${LIB_LINK})

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET MathNotes PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:MathNotes>/data)

if(MSVC)
    # CMake 3.21 is for this functionnality / Copy the dlls
    add_custom_command(TARGET MathNotes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:MathNotes> $<TARGET_FILE_DIR:MathNotes>
        COMMAND_EXPAND_LISTS
    )
endif()

# Set compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()