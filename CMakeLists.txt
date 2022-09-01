# Set C++ 17 compiler flags
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

# Set project name
project(MathNotes)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---- MicroTex ----
add_subdirectory(external/microtex)
include_directories(external/microtex)

# include_directories()

# ---- Tempo app ----
# ImGui config
add_compile_definitions(IMGUI_USE_WCHAR32)
add_subdirectory(external/TempoApp)
include_directories(external/TempoApp/include)
include_directories(external/TempoApp/external/imgui/imgui)

# Sources of program
include_directories(src)
file(GLOB source_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    src/*.cpp
    src/latex/*.cpp
)
add_executable(${PROJECT_NAME} ${source_list})
target_link_libraries(${PROJECT_NAME} PRIVATE microtex-imgui)

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET MathNotes PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:MathNotes>/data)

# Set compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W0)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()