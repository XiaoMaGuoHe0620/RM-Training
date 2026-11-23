#include "industrial_camera_ros2/mindvision_camera.hpp"
#include "CameraApi.h"
#include <chrono>
#include <iostream>

MindVisionCamera::MindVisionCamera(int idx) : device_index_(idx) {}
MindVisionCamera::~MindVisionCamera() { close(); }

bool MindVisionCamera::open() {
    tSdkCameraDevInfo dev_info[8];
    int dev_num = 8;
    if (CameraEnumerateDevice(dev_info, &dev_num) != CAMERA_STATUS_SUCCESS
        || dev_num == 0) {
        std::cerr << "[MindVision] 未枚举到设备\n"; return false;
    }
    if (device_index_ >= dev_num) return false;

    if (CameraInit(&dev_info[device_index_], -1, -1, &handle_)
        != CAMERA_STATUS_SUCCESS) return false;

    // 设置为连续采集
    CameraSetTriggerMode(handle_, 0);
    CameraSetGain(handle_, 0);  // 按需配置

    opened_ = true;
    return true;
}

bool MindVisionCamera::startGrabbing() {
    if (!opened_) return false;
    if (CameraPlay(handle_) != CAMERA_STATUS_SUCCESS) return false;
    running_ = true;
    grab_thread_ = std::thread(&MindVisionCamera::grabLoop, this);
    return true;
}

void MindVisionCamera::stopGrabbing() {
    running_ = false;
    if (grab_thread_.joinable()) grab_thread_.join();
    if (handle_ >= 0) CameraPause(handle_);
}

bool MindVisionCamera::close() {
    stopGrabbing();
    if (handle_ >= 0) {
        CameraUnInit(handle_);
        handle_ = -1;
    }
    opened_ = false;
    return true;
}

void MindVisionCamera::grabLoop() {
    while (running_) {
        unsigned char* raw = nullptr;
        tSdkFrameHead head{};
        if (CameraGetImageBuffer(handle_, &head, &raw, 1000)
            != CAMERA_STATUS_SUCCESS) continue;

        // 转换为BGR
        unsigned char* bgr = nullptr;
        CameraImageProcess(handle_, raw, &bgr, &head);

        auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        Frame f;
        f.width = head.iWidth;
        f.height = head.iHeight;
        f.encoding = "bgr8";
        f.data.assign(bgr, bgr + static_cast<size_t>(head.iWidth) * head.iHeight * 3);
        f.timestamp_ns = now_ns;
        f.frame_id = head.uiSeqNumber;

        CameraReleaseImageBuffer(handle_, raw);

        if (user_cb_) user_cb_(f);
    }
}
