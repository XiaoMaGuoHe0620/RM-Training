#include <memory>
#include <vector>
#include <optional>
#include <numeric>
#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/float64_multi_array.hpp"
#include "example_interfaces/msg/float64.hpp"

using Float64MultiArray = example_interfaces::msg::Float64MultiArray;
using Float64 = example_interfaces::msg::Float64;

// 直接复用 8.1 中 meanLength 的函数实现
std::optional<double> meanLength(const std::vector<double>& lengths) {
    if (lengths.empty()) return std::nullopt;
    double sum = std::accumulate(lengths.begin(), lengths.end(), 0.0);
    return sum / lengths.size();
}

class SubscriberNode : public rclcpp::Node {
public:
    SubscriberNode() : Node("subscriber_node") {
        // 创建发布者，用于发布均值结果，话题名为 "mean_output"
        publisher_ = this->create_publisher<Float64>("mean_output", 10);
        // 创建订阅者，订阅 "input_lengths" 话题
        subscription_ = this->create_subscription<Float64MultiArray>(
            "input_lengths", 10,
            std::bind(&SubscriberNode::topic_callback, this, std::placeholders::_1));
    }

private:
    void topic_callback(const Float64MultiArray::SharedPtr msg) {
        auto result = meanLength(msg->data);

        if (!result.has_value()) {
            // 收到空数组，只打印日志，不发布
            RCLCPP_INFO(this->get_logger(), "收到空数组，不发布均值。");
            return;
        }

        // 发布均值
        auto out_msg = Float64();
        out_msg.data = result.value();
        publisher_->publish(out_msg);
        RCLCPP_INFO(this->get_logger(), "收到数组，计算得均值: %.2f", out_msg.data);
    }

    rclcpp::Publisher<Float64>::SharedPtr publisher_;
    rclcpp::Subscription<Float64MultiArray>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SubscriberNode>());
    rclcpp::shutdown();
    return 0;
}
