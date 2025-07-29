cmake_minimum_required(VERSION 3.23)
project(imgui-graphnode LANGUAGES CXX)

include(GNUInstallDirs)

option(WITH_EXAMPLE "Build with example graph vizualization" OFF)

add_subdirectory(src)

install(EXPORT imgui_graphnodeTargets DESTINATION share/imgui_graphnode FILE imgui_graphnodeTargets.cmake)
install(
	FILES cmake/imgui_graphnodeCONFIG.cmake
	DESTINATION share/GeNBot
)