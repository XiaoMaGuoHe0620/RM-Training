#include "camera_acquire_publisher/mindvision_grabber.hpp"
#include <rclcpp/rclcpp.hpp>
#include <cstring>
#include <cstdlib>

MindVisionGrabber::MindVisionGrabber()
{
    CameraSdkInit(1);
}

MindVisionGrabber::~MindVisionGrabber()
{
    Close();
    if(rgb_buf_)
    {
        free(rgb_buf_);
        rgb_buf_ = nullptr;
    }
}

bool MindVisionGrabber::Open()
{
    if(is_opened_) return true;
    int cam_cnt = 1;
    tSdkCameraDevInfo enum_list;
    int ret = CameraEnumerateDevice(&enum_list, &cam_cnt);
    if(ret != CAMERA_STATUS_SUCCESS || cam_cnt <= 0)
    {
        RCLCPP_ERROR(rclcpp::get_logger("mindvision"), "No MindVision camera found!");
        return false;
    }

    ret = CameraInit(&enum_list, -1, -1, &hCamera_);
    if(ret != CAMERA_STATUS_SUCCESS)
    {
        RCLCPP_ERROR(rclcpp::get_logger("mindvision"), "CameraInit failed, ret:%d", ret);
        return false;
    }

    tSdkCameraCapbility cap;
    CameraGetCapability(hCamera_, &cap);
    rgb_buf_ = (unsigned char*)malloc(cap.sResolutionRange.iHeightMax * cap.sResolutionRange.iWidthMax * 3);
    if(!rgb_buf_)
    {
        CameraUnInit(hCamera_);
        hCamera_ = -1;
        return false;
    }

    if(cap.sIspCapacity.bMonoSensor)
    {
        channel_ = 1;
        CameraSetIspOutFormat(hCamera_, CAMERA_MEDIA_TYPE_MONO8);
    }
    else
    {
        channel_ = 3;
        CameraSetIspOutFormat(hCamera_, CAMERA_MEDIA_TYPE_BGR8);
    }

    CameraPlay(hCamera_);
    is_opened_ = true;
    RCLCPP_INFO(rclcpp::get_logger("mindvision"), "MindVision camera opened success");
    return true;
}

void MindVisionGrabber::Close()
{
    if(is_opened_ && hCamera_ >=0)
    {
        CameraUnInit(hCamera_);
        hCamera_ = -1;
        is_opened_ = false;
        RCLCPP_INFO(rclcpp::get_logger("mindvision"), "MindVision camera closed");
    }
}

bool MindVisionGrabber::GetFrame(sensor_msgs::msg::Image &img_msg)
{
    if(!is_opened_ || hCamera_ <0) return false;
    tSdkFrameHead frame_info;
    BYTE* p_buf = nullptr;
    int ret = CameraGetImageBuffer(hCamera_, &frame_info, &p_buf, 1000);
    if(ret != CAMERA_STATUS_SUCCESS)
    {
        return false;
    }

    CameraImageProcess(hCamera_, p_buf, rgb_buf_, &frame_info);

    img_msg.header.stamp = rclcpp::Clock().now();
    img_msg.header.frame_id = "camera";
    img_msg.height = frame_info.iHeight;
    img_msg.width = frame_info.iWidth;
    if(channel_ ==1)
    {
        img_msg.encoding = "mono8";
    }
    else
    {
        img_msg.encoding = "bgr8";
    }
    img_msg.step = img_msg.width * channel_;
    img_msg.data.resize(img_msg.height * img_msg.step);
    memcpy(img_msg.data.data(), rgb_buf_, img_msg.data.size());

    CameraReleaseImageBuffer(hCamera_, p_buf);
    return true;
}
