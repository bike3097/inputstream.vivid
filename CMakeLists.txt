cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 核心配置
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 引入 Kodi 头文件（必须保留，解决依赖问题）
include_directories(${KODI_SOURCE_DIR})
include_directories(${KODI_SOURCE_DIR}/xbmc)
include_directories(${KODI_SOURCE_DIR}/xbmc/addons)
include_directories(${KODI_SOURCE_DIR}/xbmc/cores/VideoPlayer)

# 🔥 关键修复：显式写出所有源码文件（100%不会找不到）
set(SOURCES
    main.cpp
    Utils.cpp
    VividDecoder.cpp
)

# 🔥 关键修复：显式写出所有头文件（可选，更规范）
set(HEADERS
    Utils.h
    VividDecoder.h
)

# 编译插件（无 lib 前缀，符合 Kodi 要求）
add_library(${ADDON_ID} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 系统依赖
if(UNIX)
    target_link_libraries(${ADDON_ID} pthread dl)
endif()
