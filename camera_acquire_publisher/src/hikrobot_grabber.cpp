// SPDX-License-Identifier: MIT
// 海康机器人 MVS SDK 后端。仅当 -DUSE_HIKROBOT=ON 时编译。
//
// 说明：不同 MVS 版本的枚举名/结构体字段可能略有差异，安装 SDK 后以
// /opt/MVS/include/MvCameraControl.h 中的实际声明为准；本文件给出完整可接线的
// 取流流程（枚举设备 -> 打开 -> 连续触发 -> 取帧 -> Bayer 转 RGB8 -> 释放）。
#include "camera_acquire_publisher/hikrobot_grabber.hpp"

#if USE_HIKROBOT

#include <cstring>
#include <vector>

#include "MvCameraControl.h"

namespace camera_acquire {

namespace {
// 把 MVS 像素类型翻译成 sensor_msgs 编码
std::string pixel_to_encoding(unsigned int enPixelType) {
  switch (enPixelType) {
    case PixelType_Gvsp_Mono8: return "mono8";
    case PixelType_Gvsp_BayerRG8: return "bayer_rg8";
    case PixelType_Gvsp_BayerBG8: return "bayer_bg8";
    case PixelType_Gvsp_BayerGR8: return "bayer_gr8";
    case PixelType_Gvsp_BayerGB8: return "bayer_gb8";
    case PixelType_Gvsp_RGB8Packed: return "rgb8";
    default: return "mono8";
  }
}

bool is_bayer(unsigned int enPixelType) {
  return enPixelType == PixelType_Gvsp_BayerRG8 ||
         enPixelType == PixelType_Gvsp_BayerBG8 ||
         enPixelType == PixelType_Gvsp_BayerGR8 ||
         enPixelType == PixelType_Gvsp_BayerGB8;
}
}  // namespace

HikRobotGrabber::HikRobotGrabber() = default;

HikRobotGrabber::~HikRobotGrabber() { close(); }

bool HikRobotGrabber::open(const std::string& serial) {
  // 1) 初始化 SDK
  unsigned int nRet = MV_CC_Initialize();
  if (nRet != MV_OK) return false;

  // 2) 枚举 GigE + USB3 设备
  MV_CC_DEVICE_INFO_LIST stDevList;
  std::memset(&stDevList, 0, sizeof(stDevList));
  nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDevList);
  if (nRet != MV_OK || stDevList.nDeviceNum == 0) return false;

  // 3) 按序列号匹配；serial 为空则取第一台
  const MV_CC_DEVICE_INFO* pDev = nullptr;
  for (unsigned int i = 0; i < stDevList.nDeviceNum; ++i) {
    const MV_CC_DEVICE_INFO* info = stDevList.pDeviceInfo[i];
    std::string sn;
    if (info->nTLayerType == MV_USB_DEVICE) {
      sn = reinterpret_cast<const char*>(info->SpecialInfo.stUsb3VInfo.chSerialNumber);
    } else {
      sn = reinterpret_cast<const char*>(info->SpecialInfo.stGigEInfo.chSerialNumber);
    }
    if (serial.empty() || sn == serial) { pDev = info; break; }
  }
  if (!pDev) return false;

  // 4) 创建句柄并打开设备（独占访问）
  nRet = MV_CC_CreateHandle(&handle_, pDev);
  if (nRet != MV_OK) return false;
  nRet = MV_CC_OpenDevice(handle_, 0, 0);
  if (nRet != MV_OK) { MV_CC_DestroyHandle(handle_); handle_ = nullptr; return false; }

  // 5) 连续取流（关闭触发模式）
  MV_CC_SetEnumValue(handle_, "TriggerMode", 0);
  nRet = MV_CC_StartGrabbing(handle_);
  if (nRet != MV_OK) { MV_CC_CloseDevice(handle_); MV_CC_DestroyHandle(handle_);
                       handle_ = nullptr; return false; }

  grabbing_ = true;
  opened_ = true;
  return true;
}

bool HikRobotGrabber::grab(RawFrame& frame, int timeout_ms) {
  if (!opened_ || !grabbing_) return false;

  MV_FRAME_OUT stFrame;
  std::memset(&stFrame, 0, sizeof(stFrame));
  unsigned int nRet = MV_CC_GetImageBuffer(handle_, &stFrame, timeout_ms);
  if (nRet != MV_OK) return false;

  const auto& fi = stFrame.stFrameInfo;
  frame.width = fi.nWidth;
  frame.height = fi.nHeight;
  frame.frame_id_seq = fi.nFrameNum;
  frame.hardware_timestamp_ns = fi.nTimeStamp;  // 相机硬件时间戳（ns）

  // Bayer 原图先转成 RGB8 再发布；单色图直接透传
  if (is_bayer(fi.enPixelType)) {
    MV_CC_PIXEL_CONVERT_PARAM stCvt;
    std::memset(&stCvt, 0, sizeof(stCvt));
    stCvt.enSrcPixelType = fi.enPixelType;
    stCvt.enDstPixelType = PixelType_RGB8Packed;
    stCvt.pSrcData = stFrame.pBufAddr;
    stCvt.nSrcWidth = fi.nWidth;
    stCvt.nSrcHeight = fi.nHeight;
    const size_t dstSize = static_cast<size_t>(fi.nWidth) * fi.nHeight * 3;
    frame.data.resize(dstSize);
    stCvt.pDstData = frame.data.data();
    stCvt.nDstBufSize = static_cast<unsigned int>(dstSize);
    nRet = MV_CC_ConvertPixelType(&stCvt);
    frame.encoding = (nRet == MV_OK) ? "rgb8" : pixel_to_encoding(fi.enPixelType);
  } else {
    frame.encoding = pixel_to_encoding(fi.enPixelType);
    frame.data.assign(stFrame.pBufAddr, stFrame.pBufAddr + fi.nFrameLen);
  }

  MV_CC_FreeImageBuffer(handle_, &stFrame);
  return true;
}

void HikRobotGrabber::close() {
  if (grabbing_) { MV_CC_StopGrabbing(handle_); grabbing_ = false; }
  if (opened_) {
    MV_CC_CloseDevice(handle_);
    MV_CC_DestroyHandle(handle_);
    MV_CC_Finalize();
    handle_ = nullptr;
    opened_ = false;
  }
}

}  // namespace camera_acquire
#endif  // USE_HIKROBOT
