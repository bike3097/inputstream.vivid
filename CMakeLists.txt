# CMake最低版本要求（Kodi 21要求≥3.16）
cmake_minimum_required(VERSION 3.16)

# 项目名称（与插件ID一致）
project(inputstream.vivid)

# C++标准：Kodi 21使用C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 预定义宏：Linux平台、大文件支持（>4GB）
add_definitions(-D_LINUX -D_FILE_OFFSET_BITS=64)

# 查找并包含Kodi开发头文件
find_package(Kodi REQUIRED)
include_directories(${KODI_INCLUDE_DIR})

# 进入src目录编译核心代码
add_subdirectory(src)