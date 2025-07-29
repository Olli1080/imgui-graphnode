include(CMakeFindDependencyMacro)

find_dependency(imgui)
find_dependency(PkgConfig)

pkg_search_module(graphviz libgvc IMPORTED_TARGET REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/imgui_graphnodeTargets.cmake)