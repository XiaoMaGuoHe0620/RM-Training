// SPDX-License-Identifier: MIT
#include "camera_acquire_publisher/virtual_grabber.hpp"

#include <cmath>

namespace camera_acquire {

VirtualGrabber::VirtualGrabber(uint32_t width, uint32_t height,
                               const std::string& encoding)
    : width_(width), height_(height), encoding_(encoding) {
  // 虚拟相机只保证 rgb8 / mono8 两种输出，其它编码按 rgb8 处理
  if (encoding_ != "rgb8" && encoding_ != "mono8") {
    encoding_ = "rgb8";
  }
}

bool VirtualGrabber::open(const std::string& /*serial*/) {
  opened_ = true;
  return true;
}

bool VirtualGrabber::grab(RawFrame& frame, int /*timeout_ms*/) {
  if (!opened_) return false;

  ++tick_;
  frame.frame_id_seq = tick_;
  frame.width = width_;
  frame.height = height_;
  frame.encoding = encoding_;
  frame.hardware_timestamp_ns = 0;  // 由节点用系统时间打戳

  const bool mono = (encoding_ == "mono8");
  const uint32_t channels = mono ? 1u : 3u;
  frame.data.resize(static_cast<size_t>(width_) * height_ * channels);

  // 运动目标：白色方块按正弦轨迹移动，画面每帧都在变（对应“现场画面变化时图像随之更新”）
  const double cx = width_ / 2.0 + (width_ / 2.0 - 80.0) * std::sin(tick_ * 0.05);
  const double cy = height_ / 2.0 + (height_ / 2.0 - 80.0) * std::cos(tick_ * 0.07);
  const uint32_t half = 30;

  for (uint32_t y = 0; y < height_; ++y) {
    for (uint32_t x = 0; x < width_; ++x) {
      size_t idx = (static_cast<size_t>(y) * width_ + x) * channels;
      // 背景：水平/垂直渐变 + 随时间变化的蓝色分量，保证相邻帧不同
      uint8_t r = static_cast<uint8_t>(x * 255 / width_);
      uint8_t g = static_cast<uint8_t>(y * 255 / height_);
      uint8_t b = static_cast<uint8_t>(128 + 127 * std::sin(tick_ * 0.1));
      // 运动白块
      if (x > cx - half && x < cx + half && y > cy - half && y < cy + half) {
        r = g = b = 255;
      }
      if (mono) {
        frame.data[idx] = static_cast<uint8_t>((r + g + b) / 3);
      } else {
        frame.data[idx + 0] = r;
        frame.data[idx + 1] = g;
        frame.data[idx + 2] = b;
      }
    }
  }
  return true;
}

void VirtualGrabber::close() {
  opened_ = false;
}

}  // namespace camera_acquire
