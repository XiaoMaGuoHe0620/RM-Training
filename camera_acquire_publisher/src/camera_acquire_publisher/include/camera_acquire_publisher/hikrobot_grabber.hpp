#pragma once
#include "base_grabber.hpp"
#include "MvCameraControl.h"

class HikRobotGrabber : public BaseGrabber
{
public:
    HikRobotGrabber();
    ~HikRobotGrabber() override;
    bool Open() override;
    void Close() override;
    bool GetFrame(sensor_msgs::msg::Image &img_msg) override;
private:
    void* handle_ = nullptr;
    bool opened_ = false;
    unsigned char* rgb_buf_ = nullptr;
    int buf_size_ = 0;
};
