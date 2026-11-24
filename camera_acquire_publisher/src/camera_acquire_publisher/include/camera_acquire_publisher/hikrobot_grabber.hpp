// SPDX-License-Identifier: MIT
#pragma once

#include <string>

#include "camera_acquire_publisher/grabber.hpp"

#if USE_HIKROBOT
namespace camera_acquire {

// 海康机器人（Hikrobot）MVS SDK 后端。
// 仅在编译时加 -DUSE_HIKROBOT=ON 且装好 MVS SDK 时参与编译。
class HikRobotGrabber : public CameraGrabber {
public:
  HikRobotGrabber();
  ~HikRobotGrabber() override;

  bool open(const std::string& serial) override;
  bool grab(RawFrame& frame, int timeout_ms) override;
  void close() override;

private:
  void* handle_ = nullptr;
  bool grabbing_ = false;
  bool opened_ = false;
};

}  // namespace camera_acquire
#endif  // USE_HIKROBOT
