// C++智能指针头文件，ROS2的节点，订阅者，发布者全靠std::shared_ptr管理生命周期
#include <memory>
// 动态数组容器，消息里的msg->data本质就是std::vector<double>
#include <vector>
// C++17引入的std::optional<T>：用来表示“可能有值，也可能没有值”
// 这里meanLength返回optional<double>：空数组的时候就返回无值，用来区分“均值是0”和“没有均值（空数组）”两种情况
#include <optional>
// 数值算法库，里面一般放求和，累加 std::accumulate 这类函数
#include <numeric>
// ROS2 C++核心头文件，节点，订阅，发布，日志，spin都在这里
#include "rclcpp/rclcpp.hpp"
// 引入数组消息，就是上一个发布器发送的Float64MultiArray
#include "example_interfaces/msg/float64_multi_array.hpp"
// 引入单浮点消息Float64，只有一个double data字段，用来输出计算出来的平均值
#include "example_interfaces/msg/float64.hpp"
#include "stats.h"

// 类型别名，简化长长的消息类型名字，后面直接写Float64MultiArray,Float64
using Float64MultiArray = example_interfaces::msg::Float64MultiArray;
using Float64 = example_interfaces::msg::Float64;

// 自定义节点类，继承ROS2节点基类rclcpp::Node
class SubscriberNode : public rclcpp::Node {
public:
    // 构造函数：调用父类构造，节点名字叫subscriber_node,ros2 node list可以看到这个节点名
    SubscriberNode() : Node("subscriber_node") {
        // 创建发布者，用于发布均值结果，话题名为 "mean_output" 别的节点可以订阅这个话题拿到平均值
        // 消息类型：Float64（单个double）
        // 10：Qos队列深度，最多缓存10条消息
        publisher_ = this->create_publisher<Float64>("mean_output", 10);
        // 创建订阅者，订阅 "input_lengths" 话题 和前面publisher的话题名必须完全一致
        // Qos深度：10
        // std::bind：绑定回调函数topic_callback
        // std::placeholders::_1：占位符，表示收到的消息智能指针会作为第一个参数传给回调函数
        subscription_ = this->create_subscription<Float64MultiArray>(
            "input_lengths", 10,
            std::bind(&SubscriberNode::topic_callback, this, std::placeholders::_1));
    }

private:
    // 订阅回调函数：每当收到一条话题消息，ROS2自动调用这个函数
    // 参数msg是消息的共享智能指针，用msg->访问消息内容
    void topic_callback(const Float64MultiArray::SharedPtr msg) {
        // 调用外部函数meanLength,把消息里的数组msg->data(vector<double>)传进去，返回值存入result,类型是std::optional<double>
        auto result = meanLength(msg->data);

        // result.has_value()判断optional里面有没有有效值
        // 如果没有值（！has_value()），说明是空数组，打印日志，直接return,不向外发布任何消息
        // 对应publisher轮流发空数组的场景
        if (!result.has_value()) {
            // 收到空数组，只打印日志，不发布
            RCLCPP_INFO(this->get_logger(), "收到空数组，不发布均值。");
            return;
        }

        // 发布均值
        // 新建一条Float64消息，result.value()取出optional里面存的double平均值，赋值给消息的data
        // 注意：必须先has_value()判断过，再调用.value()，否则会抛异常，这里前面已经判断，是安全的
        auto out_msg = Float64();
        out_msg.data = result.value();
        // 把均值消息发布到mean_output,打印INFO日志，%.2f保留两位小数输出均值
        publisher_->publish(out_msg);
        RCLCPP_INFO(this->get_logger(), "收到数组，计算得均值: %.2f", out_msg.data);
    }

    // 私有成员变量：
    // publisher_：均值输出的发布者智能指针
    rclcpp::Publisher<Float64>::SharedPtr publisher_;
    // subscription_：数组话题的订阅者智能指针
    rclcpp::Subscription<Float64MultiArray>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    // 初始化ROS2上下文，解析命令行参数，必须放在最前面
    rclcpp::init(argc, argv);
    // std::make_shared创建订阅节点实例
    // rclcpp::spin()自旋，阻塞在这里，持续监听话题，触发topic_callback,按下Ctrl+C才退出自旋
    rclcpp::spin(std::make_shared<SubscriberNode>());
    // 关闭ROS2，释放资源，spin退出后执行
    rclcpp::shutdown();
    return 0;
}
