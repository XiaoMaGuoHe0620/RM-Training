// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace camera_acquire {

// 一帧原始图像（取流层拿到的通用结构，节点层负责填 sensor_msgs/Image）
struct RawFrame {
  uint32_t width = 0;
  uint32_t height = 0;
  // sensor_msgs 编码名：rgb8 / bgr8 / mono8 / bayer_rg8 / bayer_bg8 等
  std::string encoding;
  std::vector<uint8_t> data;
  // 相机硬件时间戳（纳秒，来自 SDK；虚拟相机填 0，由节点用系统时间）
  uint64_t hardware_timestamp_ns = 0;
  uint64_t frame_id_seq = 0;  // 帧序号，便于排查丢帧
};

// 取流后端统一接口：虚拟相机 / 海康 MVS / 迈德威视 MVSDK 都实现它
class CameraGrabber {
public:
  virtual ~CameraGrabber() = default;

  // 打开设备；serial 为空字符串时取枚举到的第一台设备
  virtual bool open(const std::string& serial) = 0;

  // 抓取一帧；timeout_ms 超时返回 false
  virtual bool grab(RawFrame& frame, int timeout_ms) = 0;

  // 停止取流并释放设备（节点退出、重启时必须被调用）
  virtual void close() = 0;
};

}  // namespace camera_acquire
