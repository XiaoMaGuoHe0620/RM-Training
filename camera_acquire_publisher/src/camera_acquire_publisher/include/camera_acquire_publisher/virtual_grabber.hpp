// SPDX-License-Identifier: MIT
#pragma once

#include <cstdint>
#include <string>

#include "camera_acquire_publisher/grabber.hpp"

namespace camera_acquire {

// 虚拟相机：无需任何硬件与 SDK，程序生成逐帧变化的 RGB8 图像，
// 用于在没有实机时完成编译、发布、QoS、时间戳、退出释放与重启恢复的全部验证。
class VirtualGrabber : public CameraGrabber {
public:
  VirtualGrabber(uint32_t width, uint32_t height, const std::string& encoding);

  bool open(const std::string& serial) override;
  bool grab(RawFrame& frame, int timeout_ms) override;
  void close() override;

private:
  uint32_t width_;
  uint32_t height_;
  std::string encoding_;
  uint64_t tick_ = 0;
  bool opened_ = false;
};

}  // namespace camera_acquire
