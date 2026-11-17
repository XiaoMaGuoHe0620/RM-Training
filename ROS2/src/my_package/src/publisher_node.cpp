#include <chrono>
#include <memory>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/float64_multi_array.hpp"

using namespace std::chrono_literals;
using Float64MultiArray = example_interfaces::msg::Float64MultiArray;

class PublisherNode : public rclcpp::Node {
public:
    PublisherNode() : Node("publisher_node"), index_(0) {
        // 创建发布者，话题名为 "input_lengths"
        publisher_ = this->create_publisher<Float64MultiArray>("input_lengths", 10);
        // 创建定时器，每 1 秒发布一次
        timer_ = this->create_wall_timer(1000ms, std::bind(&PublisherNode::timer_callback, this));
    }

private:
    void timer_callback() {
        auto message = Float64MultiArray();
        // 准备几组测试数据，包含空数组
        std::vector<std::vector<double>> data_sets = {
            {10.0, 20.0, 30.0, 40.0}, // 正常数据
            {},                       // 空数组
            {5.0, 5.0, 5.0}           // 正常数据
        };

        // 轮流选择一组数据发布
        message.data = data_sets[index_ % data_sets.size()];
        
        if (message.data.empty()) {
            RCLCPP_INFO(this->get_logger(), "发布空数组");
        } else {
            RCLCPP_INFO(this->get_logger(), "发布数组: [%.1f, %.1f, %.1f, ...]", 
                        message.data[0], message.data[1], message.data[2]);
        }
        
        publisher_->publish(message);
        index_++;
    }

    rclcpp::Publisher<Float64MultiArray>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t index_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PublisherNode>());
    rclcpp::shutdown();
    return 0;
}
