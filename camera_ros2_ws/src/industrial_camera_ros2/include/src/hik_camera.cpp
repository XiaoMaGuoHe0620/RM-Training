#include "industrial_camera_ros2/hik_camera.hpp"
#include "MvCameraControl.h"
#include <chrono>
#include <cstring>
#include <iostream>

HikCamera::HikCamera(int idx) : device_index_(idx) {}
HikCamera::~HikCamera() { close(); }

bool HikCamera::open() {
    MV_CC_DEVICE_INFO_LIST list{};
    if (MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &list) != MV_OK
        || list.nDeviceNum == 0) {
        std::cerr << "[Hik] 未枚举到设备\n"; return false;
    }
    if (device_index_ >= static_cast<int>(list.nDeviceNum)) return false;

    if (MV_CC_CreateHandle(&handle_, list.pDeviceInfo[device_index_]) != MV_OK)
        return false;
    if (MV_CC_OpenDevice(handle_) != MV_OK) return false;

    // 关闭触发，连续取流
    MV_CC_SetEnumValue(handle_, "TriggerMode", 0);

    // 注册异步取流回调
    if (MV_CC_RegisterImageCallBack(handle_, &HikCamera::imageCallback, this)
        != MV_OK) return false;

    opened_ = true;
    return true;
}

bool HikCamera::startGrabbing() {
    if (!opened_) return false;
    grabbing_ = (MV_CC_StartGrabbing(handle_) == MV_OK);
    return grabbing_;
}

void HikCamera::stopGrabbing() {
    if (grabbing_) { MV_CC_StopGrabbing(handle_); grabbing_ = false; }
}

bool HikCamera::close() {
    stopGrabbing();
    if (handle_) {
        MV_CC_CloseDevice(handle_);
        MV_CC_DestroyHandle(handle_);
        handle_ = nullptr;
    }
    opened_ = false;
    return true;
}

void __stdcall HikCamera::imageCallback(unsigned char* pData,
                                         void* pFrameInfo, void* pUser) {
    auto* self = static_cast<HikCamera*>(pUser);
    auto* info = static_cast<MV_FRAME_OUT_INFO_EX*>(pFrameInfo);
    if (!info || info->nFrameLen == 0) return;

    // 时间戳取采集瞬间的系统时间（也可改用 info->nDevTimeStampHigh/Low）
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    self->onFrame(pData, info->nWidth, info->nHeight,
                  static_cast<uint64_t>(now_ns));
}

void HikCamera::onFrame(const unsigned char* data, uint32_t w,
                        uint32_t h, uint64_t ts_ns) {
    Frame f;
    f.width = w; f.height = h;
    // 若相机输出 Bayer，需在此处转换；为简洁这里假设已配置为输出BGR8
    f.encoding = "bgr8";
    size_t sz = static_cast<size_t>(w) * h * 3;
    f.data.assign(data, data + sz);
    f.timestamp_ns = ts_ns;
    f.frame_id = 0;  // 海康帧号可从 pFrameInfo->nFrameNum 取，此处略

    std::lock_guard<std::mutex> lk(cb_mutex_);
    if (user_cb_) user_cb_(f);
}
