// SPDX-License-Identifier: MIT
// 迈德威视 MVSDK 后端。仅当 -DUSE_MINDVISION=ON 时编译。
//
// 说明：迈德威视不同版本 SDK 的函数名有 MV_CC_*（新版 MVSDK）与 MV_*（旧版）两套，
// 安装后以 /opt/MindVision/include/MVSDK.h 实际声明为准；流程同样是
// 枚举 -> 打开 -> 连续取流 -> 取帧 -> Bayer 转 RGB8 -> 释放。
#include "camera_acquire_publisher/mindvision_grabber.hpp"

#if USE_MINDVISION

#include <cstring>
#include <vector>

#include "MVSDK.h"

namespace camera_acquire {

namespace {
std::string mv_pixel_to_encoding(unsigned int enPixelType) {
  switch (enPixelType) {
    case MV_PixelType_Mono8: return "mono8";
    case MV_PixelType_BayerRG8: return "bayer_rg8";
    case MV_PixelType_BayerBG8: return "bayer_bg8";
    case MV_PixelType_BayerGR8: return "bayer_gr8";
    case MV_PixelType_BayerGB8: return "bayer_gb8";
    case MV_PixelType_RGB8Packed: return "rgb8";
    default: return "mono8";
  }
}

bool mv_is_bayer(unsigned int enPixelType) {
  return enPixelType == MV_PixelType_BayerRG8 ||
         enPixelType == MV_PixelType_BayerBG8 ||
         enPixelType == MV_PixelType_BayerGR8 ||
         enPixelType == MV_PixelType_BayerGB8;
}
}  // namespace

MindVisionGrabber::MindVisionGrabber() = default;

MindVisionGrabber::~MindVisionGrabber() { close(); }

bool MindVisionGrabber::open(const std::string& serial) {
  // 1) 初始化 SDK
  int nRet = MV_CC_Initialize();
  if (nRet != MV_OK) return false;

  // 2) 枚举 GigE + USB 设备
  MV_CC_DEVICE_INFO_LIST stDevList;
  std::memset(&stDevList, 0, sizeof(stDevList));
  nRet = MV_CC_EnumDevices(MV_ACCESS_GIGE | MV_ACCESS_USB, &stDevList);
  if (nRet != MV_OK || stDevList.nDeviceNum == 0) return false;

  // 3) 按序列号匹配；serial 为空取第一台
  const MV_CC_DEVICE_INFO* pDev = nullptr;
  for (unsigned int i = 0; i < stDevList.nDeviceNum; ++i) {
    const MV_CC_DEVICE_INFO* info = stDevList.pDeviceInfo[i];
    std::string sn(reinterpret_cast<const char*>(info->chSerialNumber));
    if (serial.empty() || sn == serial) { pDev = info; break; }
  }
  if (!pDev) return false;

  // 4) 创建句柄并打开
  nRet = MV_CC_CreateHandle(&handle_, pDev);
  if (nRet != MV_OK) return false;
  nRet = MV_CC_OpenDevice(handle_);
  if (nRet != MV_OK) { MV_CC_DestroyHandle(handle_); handle_ = nullptr; return false; }

  // 5) 连续取流
  MV_CC_SetEnumValue(handle_, "TriggerMode", 0);
  nRet = MV_CC_StartGrabbing(handle_);
  if (nRet != MV_OK) { MV_CC_CloseDevice(handle_); MV_CC_DestroyHandle(handle_);
                       handle_ = nullptr; return false; }

  grabbing_ = true;
  opened_ = true;
  return true;
}

bool MindVisionGrabber::grab(RawFrame& frame, int timeout_ms) {
  if (!opened_ || !grabbing_) return false;

  MV_FRAME_OUT stFrame;
  std::memset(&stFrame, 0, sizeof(stFrame));
  int nRet = MV_CC_GetImageBuffer(handle_, &stFrame, timeout_ms);
  if (nRet != MV_OK) return false;

  const auto& fi = stFrame.stFrameInfo;
  frame.width = fi.nWidth;
  frame.height = fi.nHeight;
  frame.frame_id_seq = fi.nFrameNum;
  frame.hardware_timestamp_ns = fi.nTimeStamp;

  if (mv_is_bayer(fi.enPixelType)) {
    MV_CC_PIXEL_CONVERT_PARAM stCvt;
    std::memset(&stCvt, 0, sizeof(stCvt));
    stCvt.enSrcPixelType = fi.enPixelType;
    stCvt.enDstPixelType = MV_PixelType_RGB8Packed;
    stCvt.pSrcData = stFrame.pBufAddr;
    stCvt.nSrcWidth = fi.nWidth;
    stCvt.nSrcHeight = fi.nHeight;
    const size_t dstSize = static_cast<size_t>(fi.nWidth) * fi.nHeight * 3;
    frame.data.resize(dstSize);
    stCvt.pDstData = frame.data.data();
    stCvt.nDstBufSize = static_cast<unsigned int>(dstSize);
    nRet = MV_CC_ConvertPixelType(&stCvt);
    frame.encoding = (nRet == MV_OK) ? "rgb8" : mv_pixel_to_encoding(fi.enPixelType);
  } else {
    frame.encoding = mv_pixel_to_encoding(fi.enPixelType);
    frame.data.assign(stFrame.pBufAddr, stFrame.pBufAddr + fi.nFrameLen);
  }

  MV_CC_FreeImageBuffer(handle_, &stFrame);
  return true;
}

void MindVisionGrabber::close() {
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
#endif  // USE_MINDVISION
