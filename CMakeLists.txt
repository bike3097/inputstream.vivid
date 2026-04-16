cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 修复Amlogic硬解类型转换错误
add_compile_options(-fpermissive -w)

# Kodi官方头文件路径（模板标准写法）
include_directories(${KODI_SOURCE_DIR})
include_directories(${KODI_SOURCE_DIR}/xbmc/addons/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 自动搜索src源码
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 编译插件
add_library(${ADDON_ID} SHARED ${SOURCES})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

target_link_libraries(${ADDON_ID} pthread dl)
