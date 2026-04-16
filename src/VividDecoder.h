#pragma once
#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>

// 晶晨硬件解码库上下文前置声明
struct AmlVideoContext;
struct AmlAudioContext;

// 解码后帧结构（视频/音频）
struct VividFrame {
    // 帧类型：视频/音频
    enum Type {
        VIDEO,
        AUDIO
    } type;

    uint8_t* data = nullptr;     // 帧数据
    size_t size = 0;             // 数据大小
    uint64_t pts = 0;            // 显示时间戳（毫秒）
    uint64_t offset = 0;         // 文件偏移位置
    bool is_keyframe = false;     // 是否关键帧

    ~VividFrame();               // 析构：释放data内存
};

// 音视频流信息
struct StreamInfo {
    bool has_audio = true;
    uint32_t video_codec = 1;    // 视频编码（HEVC）
    uint32_t audio_codec = 1;    // 音频编码（AAC）
    uint16_t width = 1920;       // 视频宽
    uint16_t height = 1080;      // 视频高
    uint32_t fps = 25;           // 帧率
    uint32_t sample_rate = 48000;// 音频采样率
    uint8_t channels = 2;        // 音频声道数
    uint8_t bit_depth = 10;      // 位深（HDR Vivid）
};

// 晶晨HDR Vivid硬解核心类
class CVividDecoder {
public:
    CVividDecoder();             // 构造：加载libamcodec.so
    ~CVividDecoder();            // 析构：关闭解码器

    // 打开文件并初始化解码器
    bool Open(const std::string& filePath, uint64_t fileLength);
    void Close();                // 关闭释放资源

    // 送入一帧数据包进行解码
    bool ReadPacket(const uint8_t* data, size_t size, uint64_t fileOffset);
    // 获取解码后的音视频帧（阻塞50ms）
    std::shared_ptr<VividFrame> GetFrame();

    // 定位到文件偏移位置
    int64_t Seek(int64_t targetOffset, int whence);
    int64_t GetPosition() const; // 当前文件偏移
    int64_t GetLength() const;   // 文件总长度
    const StreamInfo& GetStreamInfo() const; // 获取流信息

private:
    void Reset();                // 重置解码器状态（用于Seek）
    bool ParseFrame(const uint8_t* data, size_t size, uint64_t offset); // 解析帧
    bool DecodeVideo(const uint8_t* data, size_t size, uint64_t pts, uint64_t offset);
    bool DecodeAudio(const uint8_t* data, size_t size, uint64_t pts, uint64_t offset);

private:
    bool m_opened = false;        // 是否已打开
    StreamInfo m_info;            // 流信息
    uint64_t m_fileLength = 0;    // 文件总长度
    uint64_t m_currentOffset = 0; // 当前读取位置

    // 解码帧队列（线程安全）
    std::queue<std::shared_ptr<VividFrame>> m_frameQueue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

    // 晶晨解码库句柄
    AmlVideoContext* m_vidCtx = nullptr;
    AmlAudioContext* m_audCtx = nullptr;
};