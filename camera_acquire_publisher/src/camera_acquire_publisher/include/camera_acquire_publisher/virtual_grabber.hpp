#pragma once
#include "base_grabber.hpp"
#include <opencv2/opencv.hpp>
#include <chrono>

class VirtualGrabber : public BaseGrabber
{
public:
    VirtualGrabber();
    ~VirtualGrabber() override;
    bool Open() override;
    void Close() override;
    bool GetFrame(sensor_msgs::msg::Image &img_msg) override;
private:
    cv::Mat canvas_;
    int x_ = 0;
    int dir_ = 1;
    bool opened_ = false;
    const int width_ = 640;
    const int height_ = 480;
};
