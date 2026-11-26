#include "camera_acquire_publisher/hikrobot_grabber.hpp"
#include <rclcpp/rclcpp.hpp>
#include <cstring>
#include <cstdlib>

HikRobotGrabber::HikRobotGrabber()
{
}

HikRobotGrabber::~HikRobotGrabber()
{
    Close();
    if(rgb_buf_) free(rgb_buf_);
}

bool HikRobotGrabber::Open()
{
    if(opened_) return true;
    MV_CC_DEVICE_INFO_LIST stDevList;
    memset(&stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDevList);
    if(nRet != MV_OK || stDevList.nDeviceNum == 0)
    {
        RCLCPP_ERROR(rclcpp::get_logger("hikrobot"), "No Hikrobot camera found");
        return false;
    }
    nRet = MV_CC_CreateHandle(&handle_, stDevList.pDeviceInfo[0]);
    if(nRet != MV_OK) return false;
    nRet = MV_CC_OpenDevice(handle_);
    if(nRet != MV_OK)
    {
        MV_CC_DestroyHandle(handle_);
        handle_ = nullptr;
        return false;
    }
    nRet = MV_CC_StartGrabbing(handle_);
    if(nRet != MV_OK)
    {
        MV_CC_CloseDevice(handle_);
        MV_CC_DestroyHandle(handle_);
        handle_ = nullptr;
        return false;
    }
    opened_ = true;
    RCLCPP_INFO(rclcpp::get_logger("hikrobot"), "Hikrobot camera open ok");
    return true;
}

void HikRobotGrabber::Close()
{
    if(handle_)
    {
        MV_CC_StopGrabbing(handle_);
        MV_CC_CloseDevice(handle_);
        MV_CC_DestroyHandle(handle_);
        handle_ = nullptr;
    }
    opened_ = false;
    RCLCPP_INFO(rclcpp::get_logger("hikrobot"), "Hikrobot camera closed");
}

bool HikRobotGrabber::GetFrame(sensor_msgs::msg::Image &img_msg)
{
    if(!opened_ || !handle_) return false;
    MV_FRAME_OUT stFrame = {0};
    int nRet = MV_CC_GetImageBuffer(handle_, &stFrame, 1000);
    if(nRet != MV_OK) return false;

    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
    stConvertParam.nWidth = stFrame.stFrameInfo.nWidth;
    stConvertParam.nHeight = stFrame.stFrameInfo.nHeight;
    stConvertParam.pSrcData = stFrame.pBufAddr;
    stConvertParam.nSrcDataLen = stFrame.stFrameInfo.nFrameLen;
    stConvertParam.enSrcPixelType = stFrame.stFrameInfo.enPixelType;
    stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
    if(!rgb_buf_)
    {
        buf_size_ = stFrame.stFrameInfo.nWidth * stFrame.stFrameInfo.nHeight * 3;
        rgb_buf_ = (unsigned char*)malloc(buf_size_);
    }
    stConvertParam.pDstBuffer = rgb_buf_;
    stConvertParam.nDstBufferSize = buf_size_;
    MV_CC_ConvertPixelType(handle_, &stConvertParam);

    img_msg.header.stamp = rclcpp::Clock().now();
    img_msg.header.frame_id = "camera";
    img_msg.height = stFrame.stFrameInfo.nHeight;
    img_msg.width = stFrame.stFrameInfo.nWidth;
    img_msg.encoding = "bgr8";
    img_msg.step = img_msg.width * 3;
    img_msg.data.resize(img_msg.height * img_msg.step);
    memcpy(img_msg.data.data(), rgb_buf_, img_msg.data.size());

    MV_CC_FreeImageBuffer(handle_, &stFrame);
    return true;
}
