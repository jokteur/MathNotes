set(HAVE_LOG OFF)
set(HAVE_AUTO_FONT_FIND OFF)
set(GLYPH_RENDER_TYPE OFF)

# set(BUILD_STATIC ON)
add_subdirectory(microtex)

add_library(microtex-imgui SHARED graphic_abstract.cpp)

set_target_properties(
    microtex-imgui PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${MICROTEX_API_VERSION}
    CXX_VISIBILITY_PRESET hidden
)

target_compile_definitions(microtex-imgui PRIVATE -DMICROTEX_LIBRARY)
target_include_directories(microtex-imgui PUBLIC .)
target_include_directories(microtex-imgui PUBLIC ../TempoApp/include)
target_include_directories(microtex-imgui PUBLIC ../TempoApp/external/glad)
target_include_directories(microtex-imgui PUBLIC ../MicroTeX/lib)

target_link_libraries(microtex-imgui PUBLIC Tempo microtex)
