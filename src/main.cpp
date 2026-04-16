// Kodi插件核心头文件
#include <kodi/AddonBase.h>
// InputStream插件接口头文件
#include <kodi/inputstream/InputStream.h>
// 硬解核心类
#include "VividDecoder.h"
// 工具函数
#include "Utils.h"

// Linux系统头文件（文件操作）
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>

using namespace kodi::inputstream;

// Kodi InputStream插件实现类
class CInputStreamVivid final : public CInputStream {
public:
    CInputStreamVivid() = default;
    ~CInputStreamVivid() override {
        Close(); // 确保关闭
    }

    // 打开媒体流（Kodi调用）
    bool Open(const std::string& url, const std::map<std::string, std::string>& props) override {
        std::string path = url;

        // 处理file://协议
        if (UTILS::StartsWith(path, "file://"))
            path = path.substr(7);

        // 仅支持 .mkv / .iso 文件
        bool isSupported =
            UTILS::EndsWith(path, ".mkv") ||
            UTILS::EndsWith(path, ".MKV") ||
            UTILS::EndsWith(path, ".iso") ||
            UTILS::EndsWith(path, ".ISO");

        if (!isSupported)
            return false;

        // 获取文件大小
        struct stat st{};
        if (stat(path.c_str(), &st) != 0)
            return false;

        // 打开文件（只读）
        m_fd = open(path.c_str(), O_RDONLY);
        if (m_fd < 0)
            return false;

        m_fileLength = st.st_size;
        // 初始化硬解解码器
        return m_decoder.Open(path, m_fileLength);
    }

    // 关闭流（Kodi调用）
    void Close() override {
        m_decoder.Close();
        if (m_fd >= 0) {
            close(m_fd);
            m_fd = -1;
        }
    }

    // 读取数据（Kodi调用：返回音视频帧数据）
    ssize_t Read(uint8_t* buf, size_t len) override {
        if (m_fd < 0 || m_fileLength == 0)
            return -1;

        // 从文件读取4KB数据包
        uint8_t packet[4096];
        ssize_t r = pread(m_fd, packet, sizeof(packet), m_decoder.GetPosition());
        if (r <= 0)
            return 0;

        // 送入解码器
        if (!m_decoder.ReadPacket(packet, r, m_decoder.GetPosition()))
            return 0;

        // 获取解码后的帧
        auto frame = m_decoder.GetFrame();
        if (!frame)
            return 0;

        // 复制帧数据到Kodi提供的缓冲区
        size_t copy = std::min(len, frame->size);
        memcpy(buf, frame->data, copy);
        return copy;
    }

    // 定位（Kodi调用）
    int64_t Seek(int64_t offset, int whence) override {
        return m_decoder.Seek(offset, whence);
    }

    int64_t Position() override {
        return m_decoder.GetPosition();
    }

    int64_t Length() override {
        return m_fileLength;
    }

    bool IsRealTime() override {
        return false; // 本地文件非实时流
    }

    // 向Kodi提供音视频流信息（编码、宽高、帧率等）
    StreamInfo GetStreamInfo() override {
        StreamInfo info{};
        auto& si = m_decoder.GetStreamInfo();

        // 视频信息：HEVC、4K/1080P、HDR Vivid
        info.video.codec = VIDEO_CODEC_HEVC;
        info.video.width = si.width;
        info.video.height = si.height;
        info.video.fpsRate = si.fps;
        info.video.fpsScale = 1;
        info.video.bitsPerPixel = si.bit_depth;
        info.video.colorSpace = COLOR_SPACE_BT2020;
        info.video.transferFunc = TRANSFER_FUNC_SMPTE2084;

        // 音频信息：AAC
        info.audio.codec = AUDIO_CODEC_AAC;
        info.audio.sampleRate = si.sample_rate;
        info.audio.channels = si.channels;

        return info;
    }

private:
    CVividDecoder m_decoder;    // 硬解核心对象
    int m_fd = -1;              // 文件句柄
    int64_t m_fileLength = 0;   // 文件总长度
};

// 插件入口类：Kodi加载插件时实例化
class CAddon final : public kodi::addon::CAddonBase {
public:
    // 创建插件实例（Kodi调用）
    ADDON_STATUS CreateInstance(
        int type,
        const std::string& instanceID,
        KODI_HANDLE instance,
        const std::string& version,
        KODI_HANDLE& addonInstance
    ) override {
        // 判断是否为InputStream类型请求
        if (type == ADDON_INSTANCE_INPUTSTREAM) {
            addonInstance = new CInputStreamVivid();
            return ADDON_STATUS_OK;
        }
        return ADDON_STATUS_NOT_IMPLEMENTED;
    }
};

// 宏：注册插件入口类（必须）
ADDONCREATOR(CAddon)