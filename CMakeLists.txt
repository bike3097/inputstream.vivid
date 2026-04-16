cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 插件基础配置
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ✅ 修复晶晨硬解库类型转换错误
add_compile_options(-fpermissive -w)

# ✅ 【关键】Kodi 官方头文件路径（来自你给的模板，100%找到 AddonBase.h）
include_directories(${KODI_SOURCE_DIR})
include_directories(${KODI_SOURCE_DIR}/xbmc/addons/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 自动搜索 src 目录下所有源码
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# 编译 Kodi 插件（无 lib 前缀）
add_library(${ADDON_ID} SHARED ${SOURCES})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统依赖库
target_link_libraries(${ADDON_ID} pthread dl)
