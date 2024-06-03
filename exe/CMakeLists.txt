# Copyright (c) 2024 Pyarelal Knowles, MIT License
set(SOURCE_FILES src/app.cpp src/app_impl.cpp src/glmesh.cpp src/main.cpp)
add_executable(rtrtool_exe ${SOURCE_FILES})
set_property(TARGET rtrtool_exe PROPERTY OUTPUT_NAME rtrtool)
target_include_directories(rtrtool_exe PRIVATE src)
target_link_libraries(
  rtrtool_exe
  PRIVATE rtrtool
          imgui
          implot
          args
          glad
          decodeless::writer
          mikktspace)

# TODO: presets?
# https://stackoverflow.com/questions/45955272/modern-way-to-set-compiler-flags-in-cross-platform-cmake-project
if(MSVC)
  add_compile_options(rtrtool_exe PRIVATE /W4 /WX)
else()
  target_compile_options(rtrtool_exe PRIVATE -Wall -Wextra -Wpedantic -Werror)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
      rtrtool_exe PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>
                          $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG_BACKTRACE>)
    try_compile(
      HAS_STDCXX_LIBBACKTRACE SOURCE_FROM_CONTENT
      stdc++_libbacktrace_test.cpp "int main() { return 0; }"
      LINK_LIBRARIES stdc++_libbacktrace)
    if(HAS_STDCXX_LIBBACKTRACE)
      target_link_libraries(rtrtool_exe
                            PRIVATE $<$<CONFIG:Debug>:stdc++_libbacktrace>)
    endif()
  endif()
endif()

add_compile_definitions(rtrtool_exe PUBLIC VK_NO_PROTOTYPES GLM_FORCE_XYZW_ONLY)

include(FetchContent)
FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY
    https://github.com/toge/embed.git # upstream:
                                      # https://github.com/batterycenter/embed.git
  GIT_TAG 524dba43adbd37901ff7fb978042d41acbb7b9e9)
FetchContent_MakeAvailable(battery-embed)
if(MSVC)

else()
  # ignore unused functions in battery-embed
  target_compile_options(rtrtool_exe PRIVATE -Wno-unused-function)
endif()

b_embed(rtrtool_exe fonts/Roboto-Medium.ttf)
b_embed(rtrtool_exe shaders/raster_mesh.vert)
b_embed(rtrtool_exe shaders/raster_mesh.geom)
b_embed(rtrtool_exe shaders/raster_mesh.frag)

set_target_properties(rtrtool_exe PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
                                             "${CMAKE_SOURCE_DIR}")
