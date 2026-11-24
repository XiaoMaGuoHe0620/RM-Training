// SPDX-License-Identifier: MIT
#pragma once

#include <string>

#include "camera_acquire_publisher/grabber.hpp"

#if USE_MINDVISION
namespace camera_acquire {

// 迈德威视（MindVision）MVSDK 后端。
// 仅在编译时加 -DUSE_MINDVISION=ON 且装好 MVSDK 时参与编译。
class MindVisionGrabber : public CameraGrabber {
public:
  MindVisionGrabber();
  ~MindVisionGrabber() override;

  bool open(const std::string& serial) override;
  bool grab(RawFrame& frame, int timeout_ms) override;
  void close() override;

private:
  void* handle_ = nullptr;
  bool grabbing_ = false;
  bool opened_ = false;
};

}  // namespace camera_acquire
#endif  // USE_MINDVISION
