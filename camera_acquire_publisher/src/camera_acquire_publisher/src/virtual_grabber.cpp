#include "camera_acquire_publisher/virtual_grabber.hpp"

VirtualGrabber::VirtualGrabber()
{
}

VirtualGrabber::~VirtualGrabber()
{
    Close();
}

bool VirtualGrabber::Open()
{
    if(opened_) return true;
    opened_ = true;
    RCLCPP_INFO(rclcpp::get_logger("virtual_grabber"), "Virtual camera open ok");
    return true;
}

void VirtualGrabber::Close()
{
    opened_ = false;
    RCLCPP_INFO(rclcpp::get_logger("virtual_grabber"), "Virtual camera closed");
}

bool VirtualGrabber::GetFrame(sensor_msgs::msg::Image &img_msg)
{
    if(!opened_) return false;
    canvas_ = cv::Mat::zeros(height_, width_, CV_8UC3);
    cv::rectangle(canvas_, cv::Rect(x_, 200, 60, 60), cv::Scalar(0,0,255), -1);
    x_ += dir_ * 3;
    if(x_ > width_ - 60) dir_ = -1;
    if(x_ < 0) dir_ = 1;

    img_msg.header.stamp = rclcpp::Clock().now();
    img_msg.header.frame_id = "camera";
    img_msg.height = canvas_.rows;
    img_msg.width = canvas_.cols;
    img_msg.encoding = "bgr8";
    img_msg.step = canvas_.cols * 3;
    img_msg.data.resize(canvas_.total() * canvas_.elemSize());
    memcpy(img_msg.data.data(), canvas_.data, img_msg.data.size());
    return true;
}
