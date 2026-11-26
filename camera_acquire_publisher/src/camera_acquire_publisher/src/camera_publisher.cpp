#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <memory>
#include "camera_acquire_publisher/base_grabber.hpp"

#if defined(USE_VIRTUAL)
#include "camera_acquire_publisher/virtual_grabber.hpp"
#elif defined(USE_MINDVISION)
#include "camera_acquire_publisher/mindvision_grabber.hpp"
#elif defined(USE_HIKROBOT)
#include "camera_acquire_publisher/hikrobot_grabber.hpp"
#endif

using namespace std::chrono_literals;

class CameraPubNode : public rclcpp::Node
{
public:
    CameraPubNode() : Node("camera_acquire_publisher")
    {
#if defined(USE_VIRTUAL)
        grabber_ = std::make_shared<VirtualGrabber>();
#elif defined(USE_MINDVISION)
        grabber_ = std::make_shared<MindVisionGrabber>();
#elif defined(USE_HIKROBOT)
        grabber_ = std::make_shared<HikRobotGrabber>();
#endif
        if(!grabber_->Open())
        {
            RCLCPP_ERROR(get_logger(), "Grabber open failed, exit");
            rclcpp::shutdown();
            return;
        }
        pub_ = this->create_publisher<sensor_msgs::msg::Image>("camera/image_raw", 10);
        timer_ = this->create_wall_timer(33ms, std::bind(&CameraPubNode::TimerCb, this));
    }
    ~CameraPubNode() override
    {
        if(grabber_) grabber_->Close();
    }
private:
    void TimerCb()
    {
        sensor_msgs::msg::Image img;
        if(grabber_->GetFrame(img))
        {
            pub_->publish(img);
        }
    }
    std::shared_ptr<BaseGrabber> grabber_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CameraPubNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
