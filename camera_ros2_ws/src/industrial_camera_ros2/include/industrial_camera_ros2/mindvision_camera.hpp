#pragma once
#include "industrial_camera_ros2/camera_interface.hpp"
#include <atomic>
#include <thread>

class MindVisionCamera : public CameraInterface {
public:
    explicit MindVisionCamera(int device_index = 0);
    ~MindVisionCamera() override;

    bool open() override;
    bool startGrabbing() override;
    void stopGrabbing() override;
    bool close() override;
    void setFrameCallback(FrameCallback cb) override { user_cb_ = std::move(cb); }
    std::string name() const override { return "MindVision"; }

private:
    void grabLoop();

    int device_index_;
    int handle_{-1};
    bool opened_{false};
    std::atomic<bool> running_{false};
    std::thread grab_thread_;
    FrameCallback user_cb_;
};
