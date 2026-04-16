# 定义编译目标：生成动态库inputstream.vivid.so
add_library(inputstream.vivid SHARED
  main.cpp            # 插件入口、Kodi接口实现
  VividDecoder.cpp    # 晶晨硬解核心逻辑
  Utils.cpp           # 工具函数（字符串判断）
)

# 包含头文件路径（Kodi头文件）
target_include_directories(inputstream.vivid PRIVATE
  ${KODI_INCLUDE_DIR}
)

# 链接系统库：线程、动态加载库
target_link_libraries(inputstream.vivid PRIVATE
  pthread
  dl
)

# 设置输出库属性：去掉lib前缀、后缀为.so
set_target_properties(inputstream.vivid PROPERTIES PREFIX "")
set_target_properties(inputstream.vivid PROPERTIES SUFFIX ".so")