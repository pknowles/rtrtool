# Copyright (c) 2024 Pyarelal Knowles, MIT License

set(SOURCE_FILES src/converter_gltf.cpp)
file(GLOB VS_PROJECT_HEADERS include/rtrtool/*.hpp src/*.hpp)
add_library(rtrtool ${SOURCE_FILES} ${VS_PROJECT_HEADERS})
target_include_directories(rtrtool PRIVATE src)
target_include_directories(rtrtool PUBLIC include)
target_link_libraries(rtrtool PUBLIC readytorender decodeless::writer cgltf)
target_compile_definitions(rtrtool PUBLIC GLM_ENABLE_EXPERIMENTAL
                                          GLM_FORCE_XYZW_ONLY)

# TODO: presets?
# https://stackoverflow.com/questions/45955272/modern-way-to-set-compiler-flags-in-cross-platform-cmake-project
if(MSVC)
  add_compile_options(rtrtool PRIVATE /W4 /WX)
else()
  target_compile_options(rtrtool PRIVATE -Wall -Wextra -Wpedantic -Werror)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
      rtrtool PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>
                      $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG_BACKTRACE>)
    try_compile(
      HAS_STDCXX_LIBBACKTRACE SOURCE_FROM_CONTENT
      stdc++_libbacktrace_test.cpp "int main() { return 0; }"
      LINK_LIBRARIES stdc++_libbacktrace)
    if(HAS_STDCXX_LIBBACKTRACE)
      target_link_libraries(rtrtool PUBLIC
                            $<$<CONFIG:Debug>:stdc++_libbacktrace>)
    endif()
  endif()
endif()

# Hacks to interface with KTX imageio. Maybe they forgot to export some include
# directories or forgot to PIMPL non-public headers? There's also a duplicate
# copy of glm that gets exported from 'other_include'
add_library(rtrtool_ktx src/rtrtool_ktx.cpp src/rtrtool_ktx.hpp)
target_link_libraries(rtrtool_ktx PRIVATE decodeless::writer imageio ktx)
target_include_directories(rtrtool_ktx PRIVATE src include)
target_include_directories(
  rtrtool_ktx
  PRIVATE ../external/KTX-Software/utils
          ../external/KTX-Software/include
          ../external/KTX-Software/tools/ktx
          ../external/KTX-Software/other_projects/cxxopts/include
          ../external/KTX-Software/lib
          ../external/KTX-Software/lib/astc-encoder/Source
          ../external/KTX-Software/lib/basisu
          ../external/KTX-Software/lib/dfdutils)
set_source_files_properties(
  src/rtrtool_ktx.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-pedantic -Wno-deprecated-enum-enum-conversion")

target_link_libraries(rtrtool PUBLIC rtrtool_ktx)

install(TARGETS rtrtool)
