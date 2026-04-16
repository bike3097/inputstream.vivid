cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 修复晶晨硬解库类型转换报错
add_compile_options(-fpermissive -w)

# --------------------------
# 🔥 模板官方精准路径（唯一能找到 AddonBase.h 的写法）
# --------------------------
include_directories(${KODI_SOURCE_DIR}/xbmc/addons/include)
include_directories(src)

# 自动搜索src源码
file(GLOB_RECURSE SOURCES src/*.cpp)

# 编译插件
add_library(${ADDON_ID} SHARED ${SOURCES})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统依赖
target_link_libraries(${ADDON_ID} pthread dl)
