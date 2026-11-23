#pragma once
#include "industrial_camera_ros2/camera_interface.hpp"
#include <atomic>
#include <mutex>

class HikCamera : public CameraInterface {
public:
    explicit HikCamera(int device_index = 0);
    ~HikCamera() override;

    bool open() override;
    bool startGrabbing() override;
    void stopGrabbing() override;
    bool close() override;
    void setFrameCallback(FrameCallback cb) override { user_cb_ = std::move(cb); }
    std::string name() const override { return "Hikvision"; }

private:
    static void __stdcall imageCallback(unsigned char* pData,
                                         void* pFrameInfo, void* pUser);
    void onFrame(const unsigned char* data, uint32_t w, uint32_t h,
                 uint64_t frame_id);

    int device_index_;
    void* handle_{nullptr};
    bool opened_{false};
    bool grabbing_{false};
    FrameCallback user_cb_;
    std::mutex cb_mutex_;
};
