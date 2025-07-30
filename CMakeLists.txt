cmake_minimum_required(VERSION 3.23)
project(imgui-graphnode LANGUAGES CXX)

include(GNUInstallDirs)

option(WITH_EXAMPLE "Build with example graph vizualization" ON)

add_subdirectory(src)

install(EXPORT imgui-graphnodeTargets DESTINATION share/imgui-graphnode FILE imgui-graphnodeTargets.cmake)
install(
	FILES cmake/imgui-graphnodeConfig.cmake
	DESTINATION share/imgui-graphnode
)