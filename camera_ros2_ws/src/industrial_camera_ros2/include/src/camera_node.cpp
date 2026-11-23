#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include "industrial_camera_ros2/hik_camera.hpp"
#include "industrial_camera_ros2/mindvision_camera.hpp"

class CameraNode : public rclcpp::Node {
public:
    CameraNode() : Node("industrial_camera_node") {
        vendor_ = declare_parameter<std::string>("vendor", "hikvision");
        device_index_ = declare_parameter<int>("device_index", 0);
        frame_id_ = declare_parameter<std::string>("frame_id", "camera_optical_frame");
        topic_ = declare_parameter<std::string>("topic", "image_raw");

        // 工业相机图像流：Best Effort + 队列深度1，保证低延迟
        auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
                       .best_effort()
                       .durability_volatile();
        pub_ = create_publisher<sensor_msgs::msg::Image>(topic_, qos);

        if (vendor_ == "hikvision") {
            camera_ = std::make_unique<HikCamera>(device_index_);
        } else if (vendor_ == "mindvision") {
            camera_ = std::make_unique<MindVisionCamera>(device_index_);
        } else {
            RCLCPP_FATAL(get_logger(), "不支持的相机厂商: %s", vendor_.c_str());
            throw std::runtime_error("unsupported vendor");
        }

        camera_->setFrameCallback(
            [this](const Frame& f) { publishFrame(f); });

        if (!camera_->open()) {
            RCLCPP_FATAL(get_logger(), "打开相机失败");
            throw std::runtime_error("open failed");
        }
        if (!camera_->startGrabbing()) {
            RCLCPP_FATAL(get_logger(), "启动取流失败");
            throw std::runtime_error("start grabbing failed");
        }
        RCLCPP_INFO(get_logger(), "相机 [%s] 已启动，发布话题: /%s",
                    camera_->name().c_str(), topic_.c_str());
    }

    ~CameraNode() override {
        if (camera_) camera_->close();  // 正常退出释放相机
    }

private:
    void publishFrame(const Frame& f) {
        auto msg = sensor_msgs::msg::Image();
        msg.header.stamp = rclcpp::Time(f.timestamp_ns);
        msg.header.frame_id = frame_id_;
        msg.height = f.height;
        msg.width = f.width;
        msg.encoding = f.encoding;
        msg.is_bigendian = 0;
        msg.step = f.width * (f.encoding == "bgr8" ? 3 : 1);
        msg.data = f.data;
        pub_->publish(msg);
    }

    std::string vendor_, frame_id_, topic_;
    int device_index_{0};
    std::unique_ptr<CameraInterface> camera_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    try {
        rclcpp::spin(std::make_shared<CameraNode>());
    } catch (const std::exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("main"), "%s", e.what());
    }
    rclcpp::shutdown();
    return 0;
}
