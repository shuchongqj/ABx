cmake_minimum_required (VERSION 3.17)
project (abserv CXX)

# For stdafx.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/abserv)

file(GLOB_RECURSE ABSERV_SOURCES      abserv/*.cpp abserv/*.h 
                                      abserv/actions/*.cpp abserv/actions/*.h
                                      abserv/conditions/*.cpp abserv/conditions/*.h
                                      abserv/filters/*.cpp abserv/filters/*.h)

add_executable(
    abserv
    ${ABSERV_SOURCES}
)

if (ABX_SCENE_VIEWER)
  target_compile_definitions(abserv PRIVATE SCENE_VIEWER)
endif(ABX_SCENE_VIEWER)

target_link_libraries(abserv lua abscommon abcrypto absmath abai abipc abshared pugixml Detour DetourTileCache EASTL PugiXml)