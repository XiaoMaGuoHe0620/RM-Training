#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct Frame {
    std::vector<uint8_t> data;   // 原始像素数据
    uint32_t width{0};
    uint32_t height{0};
    std::string encoding;        // 如 "bgr8" / "mono8"
    uint64_t frame_id{0};
    uint64_t timestamp_ns{0};    // 采集时间，由SDK提供或采集瞬间取系统时间
};

using FrameCallback = std::function<void(const Frame&)>;

class CameraInterface {
public:
    virtual ~CameraInterface() = default;
    virtual bool open() = 0;
    virtual bool startGrabbing() = 0;
    virtual void stopGrabbing() = 0;
    virtual bool close() = 0;
    virtual void setFrameCallback(FrameCallback cb) = 0;
    virtual std::string name() const = 0;
};
