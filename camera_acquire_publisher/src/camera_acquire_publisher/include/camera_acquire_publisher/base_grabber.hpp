#pragma once
#include <sensor_msgs/msg/image.hpp>
#include <rclcpp/rclcpp.hpp>

class BaseGrabber
{
public:
    virtual ~BaseGrabber() = default;
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual bool GetFrame(sensor_msgs::msg::Image &img_msg) = 0;
};
