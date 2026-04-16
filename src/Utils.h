#pragma once
#include <string>

namespace UTILS {
    // 判断字符串是否以指定前缀开头
    bool StartsWith(const std::string& str, const std::string& prefix);
    // 判断字符串是否以指定后缀结尾
    bool EndsWith(const std::string& str, const std::string& suffix);
}