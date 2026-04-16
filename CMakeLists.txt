cmake_minimum_required(VERSION 3.16)
project(inputstream.vivid)

# 插件ID（必须和addon.xml一致）
set(ADDON_ID inputstream.vivid)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 核心：直接引用Kodi源码头文件（解决FindKodi.cmake报错）
include_directories(${KODI_SOURCE_DIR}/xbmc)
include_directories(${KODI_SOURCE_DIR}/xbmc/addons)
include_directories(${KODI_SOURCE_DIR}/xbmc/cores)
include_directories(${KODI_SOURCE_DIR}/xbmc/utils)

# 编译源码
file(GLOB SOURCES
    main.cpp
    Utils.cpp
    VividDecoder.cpp
)

# 生成插件二进制（Kodi要求无lib前缀）
add_library(${ADDON_ID} SHARED ${SOURCES})

# 禁用前缀（关键！生成 inputstream.vivid.so 而非 libinputstream.vivid.so）
set_target_properties(${ADDON_ID} PROPERTIES PREFIX "")

# 链接依赖
if(UNIX)
    target_link_libraries(${ADDON_ID} pthread)
endif()
