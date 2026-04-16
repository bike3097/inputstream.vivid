cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 插件配置
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ✅ 修复1：添加编译参数，解决晶晨库类型转换错误
add_compile_options(-fpermissive -w)

# ✅ 修复2：正确包含Kodi 21所有头文件（解决AddonBase.h找不到）
include_directories(${KODI_SOURCE_DIR})
include_directories(${KODI_SOURCE_DIR}/xbmc)
include_directories(${KODI_SOURCE_DIR}/xbmc/addons)
include_directories(${KODI_SOURCE_DIR}/xbmc/kodi)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 自动搜索src目录所有源码
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# 编译插件
add_library(${ADDON_ID} SHARED ${SOURCES})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统依赖
target_link_libraries(${ADDON_ID} pthread dl)
