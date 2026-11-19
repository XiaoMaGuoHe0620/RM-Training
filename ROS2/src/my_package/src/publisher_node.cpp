// 引入C++标准时间库，后面用来写定时器时间（1000ms这种字面量）
#include <chrono>
// 引入智能指针库，ROS2大量用shared_ptr共享指针管理节点，发布者，定时器对象，自动释放内存
#include <memory>
#include <vector>
// ROS2 C++核心头文件，节点，发布，订阅，日志，spin等基础API都在这里
#include "rclcpp/rclcpp.hpp"
// 引用消息定义头文件：Float64MultiArray是ROS2自带接口，代表一组double浮点数数组，里面成员data就是std::vector<double>
#include "example_interfaces/msg/float64_multi_array.hpp"

// 启用C++的时间字面量，允许直接写1000ms,1s,等价于std::chrono::milliseconds(1000)
using namespace std::chrono_literals;
// 类型别名，给长长的消息类型起短名字Float64MultiArray，简化后面代码书写
using Float64MultiArray = example_interfaces::msg::Float64MultiArray;

// 定义一个节点类PublisherNode,公有继承ROS2基类rclcpp::Node,ROS2自定义节点一般都继承rclcpp::Node
class PublisherNode : public rclcpp::Node {
public:
    // Node("publisher_node")：调用父类构造，设置节点名字为publisher_node（ros2 node list可以看到）
    // index_(0)：初始化成员变量index_从0开始，用来轮流取测试数据集
    PublisherNode() : Node("publisher_node"), index_(0) {
        // 创建发布者，话题名为 "input_lengths"
        // this->create_publisher<消息类型>(话题名,Qos队列深度)
        // 话题名称：input_lengths,订阅方要和这个名字一致才能收到
        // 10：Qos队列大小，消息堆积最多缓存10条，超过旧消息丢弃
        publisher_ = this->create_publisher<Float64MultiArray>("input_lengths", 10);
        // 创建墙钟定时器，每 1 秒发布一次
        // 1000ms：周期1s,每隔1s执行一次回调
        // std::bind(&PublisherNode::timer_callback, this)：绑定回调函数timer_callback,this是当前节点实例
        // 注意：定时器回调在spin的线程里执行，不要放长时间阻塞代码！
        timer_ = this->create_wall_timer(1000ms, std::bind(&PublisherNode::timer_callback, this));
    }

private:
    // 定时器回调函数，每1s自动进入这个函数，在这里组装消息+发布
    void timer_callback() {
        // 创建一个消息实例message,类型就是浮点数多数组消息
        // 消息结构体里最重要成员：message.data,类型std::vector<double>
        auto message = Float64MultiArray();
        // 准备几组测试数据，包含空数组
        // data_sets是二维vector：里面装了3组vector,轮流发送
        std::vector<std::vector<double>> data_sets = {
            {10.0, 20.0, 30.0, 40.0}, // 正常数据
            {},                       // 空数组
            {5.0, 5.0, 5.0}           // 正常数据
        };
        
        // 轮流选择一组数据发布
        // index_ % 3取摸运算：index依次0,1,2,0,1,2,0...循环取三组数据，实现轮流发送
        // 把选中的vector赋值给消息的data字段
        message.data = data_sets[index_ % data_sets.size()];
        
        // 判断vector是否为空：.empty()  
        // RCLCPP_INFO：ROS2标准日志打印（INFO级别，绿色）
        // this->get_logger()：获取节点的日志对象
        if (message.data.empty()) {
            RCLCPP_INFO(this->get_logger(), "发布空数组");
        } else {
            // 如果数组不为空，打印前3个元素，%.1f保留1位小数
            RCLCPP_INFO(this->get_logger(), "发布数组: [%.1f, %.1f, %.1f, ...]", 
                        message.data[0], message.data[1], message.data[2]);
        }
        
        // 调用发布者的publish()，把构造好的消息发送到ROS2话题input_lengths
        publisher_->publish(message);
        // 序号+1,下一轮取下一组数据
        index_++;
    }

    // 类的私有成员变量
    // publisher_:发布者智能指针
    rclcpp::Publisher<Float64MultiArray>::SharedPtr publisher_;
    // timer_：定时器智能指针
    rclcpp::TimerBase::SharedPtr timer_;
    // index_:无符号整数，循环索引，size_t适合数组下标
    size_t index_;
};

int main(int argc, char * argv[]) {
    // ROS2初始化，必须写！解析命令行参数，初始化ROS2上下文
    rclcpp::init(argc, argv);
    // std::make_shared<PublisherNode>()：创建节点对象的shared_ptr,自动调用构造函数
    // rclcpp::spin(节点)：进入自旋循环！spin会不停处理节点的定时器，订阅回调，阻塞在这里，直到Ctrl+C终止程序
    // 定时器回调就是spin线程触发执行的
    rclcpp::spin(std::make_shared<PublisherNode>());
    // ROS2关闭，释放资源，在spin退出后执行
    rclcpp::shutdown();
    return 0;
}
