#include "VividDecoder.h"
#include <cstring>
#include <algorithm>
#include <dlfcn.h>
#include <chrono>

// 全局：晶晨硬件解码库函数指针（单例加载）
static struct {
    void* handle = nullptr;       // libamcodec.so句柄

    // 视频解码函数
    void* (*vid_new)() = nullptr;
    bool (*vid_open)(void*, int w, int h, int codec) = nullptr;
    bool (*vid_decode)(void*, const uint8_t*, size_t, uint64_t, bool) = nullptr;
    bool (*vid_get)(void*, uint8_t**, size_t*, uint64_t*) = nullptr;
    void (*vid_close)(void*) = nullptr;

    // 音频解码函数
    void* (*aud_new)() = nullptr;
    bool (*aud_open)(void*, int sr, int ch, int codec) = nullptr;
    bool (*aud_decode)(void*, const uint8_t*, size_t, uint64_t) = nullptr;
    bool (*aud_get)(void*, uint8_t**, size_t*, uint64_t*) = nullptr;
    void (*aud_close)(void*) = nullptr;

    bool inited = false;
} g_aml;

// VividFrame析构：释放帧数据
VividFrame::~VividFrame() {
    if (data) delete[] data;
}

// 构造：加载晶晨解码库并获取函数指针
CVividDecoder::CVividDecoder() {
    if (!g_aml.inited) {
        // 动态加载晶晨硬件解码库
        g_aml.handle = dlopen("libamcodec.so", RTLD_LAZY);
        if (g_aml.handle) {
            // 获取视频解码函数
            g_aml.vid_new = (void*(*)())dlsym(g_aml.handle, "aml_vid_new");
            g_aml.vid_open = (bool(*)(void*,int,int,int))dlsym(g_aml.handle, "aml_vid_open");
            g_aml.vid_decode = (bool(*)(void*,const uint8_t*,size_t,uint64_t,bool))dlsym(g_aml.handle, "aml_vid_decode");
            g_aml.vid_get = (bool(*)(void*,uint8_t**,size_t*,uint64_t*))dlsym(g_aml.handle, "aml_vid_get");
            g_aml.vid_close = (void(*)(void*))dlsym(g_aml.handle, "aml_vid_close");

            // 获取音频解码函数
            g_aml.aud_new = (void*(*)())dlsym(g_aml.handle, "aml_aud_new");
            g_aml.aud_open = (bool(*)(void*,int,int,int))dlsym(g_aml.handle, "aml_aud_open");
            g_aml.aud_decode = (bool(*)(void*,const uint8_t*,size_t,uint64_t))dlsym(g_aml.handle, "aml_aud_decode");
            g_aml.aud_get = (bool(*)(void*,uint8_t**,size_t*,uint64_t*))dlsym(g_aml.handle, "aml_aud_get");
            g_aml.aud_close = (void(*)(void*))dlsym(g_aml.handle, "aml_aud_close");

            g_aml.inited = true;
        }
    }
}

CVividDecoder::~CVividDecoder() {
    Close();
}

// 重置解码器（清空队列、重启解码）
void CVividDecoder::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // 清空帧队列
    while (!m_frameQueue.empty())
        m_frameQueue.pop();

    m_currentOffset = 0;

    // 关闭旧解码器
    if (m_vidCtx && g_aml.vid_close)
        g_aml.vid_close(m_vidCtx);
    if (m_audCtx && g_aml.aud_close)
        g_aml.aud_close(m_audCtx);

    // 创建新解码器并打开
    m_vidCtx = g_aml.vid_new ? g_aml.vid_new() : nullptr;
    m_audCtx = g_aml.aud_new ? g_aml.aud_new() : nullptr;

    g_aml.vid_open(m_vidCtx, m_info.width, m_info.height, m_info.video_codec);
    g_aml.aud_open(m_audCtx, m_info.sample_rate, m_info.channels, m_info.audio_codec);
}

// 打开文件（仅初始化，不实际打开文件句柄）
bool CVividDecoder::Open(const std::string&, uint64_t fileLength) {
    Close();
    m_fileLength = fileLength;
    m_opened = true;
    Reset();
    return true;
}

// 关闭解码器，释放资源
void CVividDecoder::Close() {
    if (!m_opened) return;

    // 关闭音视频解码上下文
    if (m_vidCtx && g_aml.vid_close)
        g_aml.vid_close(m_vidCtx);
    if (m_audCtx && g_aml.aud_close)
        g_aml.aud_close(m_audCtx);

    m_vidCtx = nullptr;
    m_audCtx = nullptr;

    // 清空帧队列
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_frameQueue.empty())
            m_frameQueue.pop();
    }

    m_opened = false;
}

// 解析自定义帧格式（前16字节：类型、大小、PTS）
bool CVividDecoder::ParseFrame(const uint8_t* data, size_t size, uint64_t offset) {
    if (size < 16) return false;

    // 解析帧头
    uint32_t frameType = *(const uint32_t*)data;
    uint32_t frameSize = *(const uint32_t*)(data + 4);
    uint64_t pts = *(const uint64_t*)(data + 8);

    if (frameSize + 16 > size)
        return false;

    const uint8_t* payload = data + 16;

    // 根据类型送入视频/音频解码器
    if (frameType == 'VIDE')
        DecodeVideo(payload, frameSize, pts, offset);
    if (frameType == 'AUDI')
        DecodeAudio(payload, frameSize, pts, offset);

    // 更新当前文件偏移
    m_currentOffset = offset + 16 + frameSize;
    return true;
}

// 视频硬解：送入数据 → 解码 → 获取输出帧 → 入队列
bool CVividDecoder::DecodeVideo(const uint8_t* data, size_t size, uint64_t pts, uint64_t offset) {
    if (!m_vidCtx || !g_aml.vid_decode)
        return false;

    // 送入晶晨视频解码器
    g_aml.vid_decode(m_vidCtx, data, size, pts, (size > 100));

    uint8_t* outData;
    size_t outSize;
    uint64_t outPts;

    // 获取解码后视频帧
    if (g_aml.vid_get(m_vidCtx, &outData, &outSize, &outPts)) {
        auto frame = std::make_shared<VividFrame>();
        frame->type = VividFrame::VIDEO;
        frame->data = new uint8_t[outSize];
        memcpy(frame->data, outData, outSize);
        frame->size = outSize;
        frame->pts = outPts;
        frame->offset = offset;

        // 加入帧队列（线程安全）
        std::lock_guard<std::mutex> lock(m_mutex);
        m_frameQueue.push(frame);
    }
    return true;
}

// 音频硬解：送入数据 → 解码 → 获取输出帧 → 入队列
bool CVividDecoder::DecodeAudio(const uint8_t* data, size_t size, uint64_t pts, uint64_t offset) {
    if (!m_audCtx || !g_aml.aud_decode)
        return false;

    // 送入晶晨音频解码器
    g_aml.aud_decode(m_audCtx, data, size, pts);

    uint8_t* outData;
    size_t outSize;
    uint64_t outPts;

    // 获取解码后音频帧
    if (g_aml.aud_get(m_audCtx, &outData, &outSize, &outPts)) {
        auto frame = std::make_shared<VividFrame>();
        frame->type = VividFrame::AUDIO;
        frame->data = new uint8_t[outSize];
        memcpy(frame->data, outData, outSize);
        frame->size = outSize;
        frame->pts = outPts;
        frame->offset = offset;

        // 加入帧队列（线程安全）
        std::lock_guard<std::mutex> lock(m_mutex);
        m_frameQueue.push(frame);
    }
    return true;
}

// 外部接口：送入数据包
bool CVividDecoder::ReadPacket(const uint8_t* data, size_t size, uint64_t offset) {
    return m_opened && ParseFrame(data, size, offset);
}

// 外部接口：获取一帧（最多等待50ms）
std::shared_ptr<VividFrame> CVividDecoder::GetFrame() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait_for(lock, std::chrono::milliseconds(50));

    if (m_frameQueue.empty())
        return nullptr;

    auto frame = m_frameQueue.front();
    m_frameQueue.pop();
    return frame;
}

// 定位文件偏移（Seek）
int64_t CVividDecoder::Seek(int64_t target, int whence) {
    if (!m_opened) return -1;

    // 计算目标偏移（SEEK_SET/CUR/END）
    int64_t base = 0;
    switch (whence) {
        case SEEK_SET: base = 0; break;
        case SEEK_CUR: base = m_currentOffset; break;
        case SEEK_END: base = m_fileLength; break;
        default: return -1;
    }

    int64_t want = base + target;
    want = std::max<int64_t>(want, 0);
    want = std::min<int64_t>(want, m_fileLength);

    // 重置解码器并跳到目标位置
    Reset();
    m_currentOffset = want;
    return want;
}

int64_t CVividDecoder::GetPosition() const {
    return m_currentOffset;
}

int64_t CVividDecoder::GetLength() const {
    return m_fileLength;
}

const StreamInfo& CVividDecoder::GetStreamInfo() const {
    return m_info;
}