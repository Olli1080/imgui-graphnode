add_library(imgui_graphnode)

find_package(imgui CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(graphviz libgvc IMPORTED_TARGET REQUIRED)

target_include_directories(imgui_graphnode PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

file(RELATIVE_PATH Relative ${PROJECT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${Relative}")

target_sources(imgui_graphnode
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_graphnode_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_graphnode.cpp
    ${INCLUDE_DIR}/imgui_graphnode_internal.h
  PUBLIC FILE_SET HEADERS
  BASE_DIRS ${INCLUDE_DIR}
  FILES
    ${INCLUDE_DIR}/imgui_graphnode.h
)

if (WITH_EXAMPLE)
    target_sources(imgui_graphnode
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui_graphnode_demo.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${INCLUDE_DIR}
    FILES
        ${INCLUDE_DIR}/imgui_graphnode_demo.h
)
endif()

target_compile_features(imgui_graphnode PUBLIC cxx_std_20)
target_link_libraries(imgui_graphnode
  PUBLIC
    imgui::imgui
    PkgConfig::graphviz
)

install(
  TARGETS 
    imgui_graphnode
  EXPORT 
    imgui_graphnodeTargets
  LIBRARY
    DESTINATION lib
  FILE_SET HEADERS
    DESTINATION include/${Relative}
)