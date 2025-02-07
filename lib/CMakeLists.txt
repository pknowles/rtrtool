# Copyright (c) 2024-2025 Pyarelal Knowles, MIT License

set(SOURCE_FILES src/converter_gltf.cpp)
file(GLOB VS_PROJECT_HEADERS include/rtrtool/*.hpp src/*.hpp)
add_library(rtrtool ${SOURCE_FILES} ${VS_PROJECT_HEADERS})
target_include_directories(rtrtool PRIVATE src)
target_include_directories(rtrtool PUBLIC include)
target_link_libraries(rtrtool PUBLIC readytorender decodeless::writer cgltf)
target_compile_definitions(rtrtool PUBLIC GLM_ENABLE_EXPERIMENTAL
                                          GLM_FORCE_XYZW_ONLY)

if(MSVC)
  add_compile_options(rtrtool PRIVATE /W4 /WX)
else()
  target_compile_options(rtrtool PRIVATE -Wall -Wextra -Wpedantic -Werror)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(rtrtool PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>)

    # Ugly detection for a working _GLIBCXX_DEBUG_BACKTRACE config, but the
    # feature itself is useful
    include(../external/readytorender/test/glibcxx_debug_backtrace.cmake)
    if(GLIBCXX_DEBUG_BACKTRACE_SUPPORTED)
      target_compile_definitions(
        rtrtool PRIVATE $<$<CONFIG:Debug>:_GLIBCXX_DEBUG_BACKTRACE>)
      target_link_libraries(
        rtrtool PUBLIC $<$<CONFIG:Debug>:${GLIBCXX_DEBUG_BACKTRACE_LIBRARY}>)
      target_compile_features(rtrtool
                              PRIVATE ${GLIBCXX_DEBUG_BACKTRACE_CXX_FEATURE})
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
