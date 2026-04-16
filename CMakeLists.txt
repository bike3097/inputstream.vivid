cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 插件基础配置
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Kodi 21 头文件路径
include_directories(${KODI_SOURCE_DIR}/xbmc)
include_directories(${KODI_SOURCE_DIR}/xbmc/addons)
include_directories(${CMAKE_SOURCE_DIR}/src)

# ✅ 核心修复：自动搜索 src 目录下的所有源码文件
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# 编译插件（Kodi 规范：无 lib 前缀）
add_library(${ADDON_ID} SHARED ${SOURCES})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统依赖库
target_link_libraries(${ADDON_ID} pthread dl)
