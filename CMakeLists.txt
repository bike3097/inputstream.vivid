cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 插件配置
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 核心：Kodi 头文件路径
include_directories(${KODI_SOURCE_DIR})
include_directories(${KODI_SOURCE_DIR}/xbmc)
include_directories(${KODI_SOURCE_DIR}/xbmc/addons)

# ✅ 终极修复：用 PROJECT_SOURCE_DIR 绝对路径指定源码（永远不会找不到）
set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/Utils.cpp
    ${PROJECT_SOURCE_DIR}/VividDecoder.cpp
)

set(HEADERS
    ${PROJECT_SOURCE_DIR}/Utils.h
    ${PROJECT_SOURCE_DIR}/VividDecoder.h
)

# 编译动态库
add_library(${ADDON_ID} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统库依赖
if(UNIX)
    target_link_libraries(${ADDON_ID} pthread dl)
endif()
