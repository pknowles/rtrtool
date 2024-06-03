# Copyright (c) 2024 Pyarelal Knowles, MIT License
include(FetchContent)

add_subdirectory(decodeless/mappedfile)
add_subdirectory(decodeless/allocator)
add_subdirectory(readytorender)

# Must be after readytorender as this depends on decodeless::offset_ptr, which is
# provided via readytorender submodules
add_subdirectory(decodeless/writer)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    #${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# implot library
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(implot)
set(IMPLOT_DIR implot)
add_library(implot
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# args.hxx argument parser
FetchContent_Declare(
    args
    GIT_REPOSITORY https://github.com/Taywee/args.git
    GIT_TAG  6.4.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(args)

# glad library
set(GLAD_DIR glad)
add_library(glad
    ${GLAD_DIR}/src/gl.c
)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# cgltf
FetchContent_Declare(
    cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG v1.13
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(cgltf)
add_library(cgltf cgltf_impl.cpp)
target_include_directories(cgltf PUBLIC ${cgltf_SOURCE_DIR})

# MikkTSpace
FetchContent_Declare(
    mikktspace
    GIT_REPOSITORY https://github.com/mmikk/MikkTSpace.git
    GIT_TAG 3e895b49d05ea07e4c2133156cfa94369e19e409
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(mikktspace)
add_library(mikktspace
    ${mikktspace_SOURCE_DIR}/mikktspace.c
    ${mikktspace_SOURCE_DIR}/mikktspace.h
    )
target_include_directories(mikktspace PUBLIC ${mikktspace_SOURCE_DIR})

# KTX for writing files
#set(STATIC_APP_LIB_SYMBOL_VISIBILITY hidden)
#set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software)
#add_subdirectory(KTX-Software/tools/imageio)
set(KTX_FEATURE_TESTS OFF)
add_subdirectory(KTX-Software)
