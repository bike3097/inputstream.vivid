#include "Utils.h"

namespace UTILS {

    // 实现：前缀判断
    bool StartsWith(const std::string& str, const std::string& prefix) {
        // 比较前prefix.size()个字符
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    // 实现：后缀判断
    bool EndsWith(const std::string& str, const std::string& suffix) {
        if (str.size() < suffix.size())
            return false;
        // 取末尾suffix.size()个字符比较
        return str.substr(str.size() - suffix.size()) == suffix;
    }

}