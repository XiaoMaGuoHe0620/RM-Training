#pragma once
#include "base_grabber.hpp"
#include "CameraApi.h"

class MindVisionGrabber : public BaseGrabber
{
public:
    MindVisionGrabber();
    ~MindVisionGrabber() override;
    bool Open() override;
    void Close() override;
    bool GetFrame(sensor_msgs::msg::Image &img_msg) override;
private:
    int hCamera_ = -1;
    bool is_opened_ = false;
    unsigned char* rgb_buf_ = nullptr;
    int channel_ = 3;
};
