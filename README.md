# ROS2 均值计算实验包 my_package
## 项目说明
本包包含两个ROS2节点：发布节点循环发送浮点数组（包含空数组），订阅节点接收数组，调用静态库函数计算均值；空数组时不发布均值。

## 一、构建和运行命令
```bash
# 1. 清理并重新编译
cd ~/my-notes/ROS2
rm -rf build install log
colcon build --packages-select my_package

# 2. 验证可执行文件
ls install/my_package/lib/my_package/

# 3. 确认环境变量
source install/setup.bash
echo $ROS_PACKAGE_PATH

# 【终端1】启动发布节点
ros2 run my_package publisher_node

# 【终端2】启动订阅节点
ros2 run my_package subscriber_node
```

## 二、节点、话题、消息类型与输入输出
| 节点名          | 作用                           | 话题名         | 消息类型                                 |
| --------------- | ------------------------------ | -------------- | ---------------------------------------- |
| publisher_node  | 循环发布数组                   | /input_lengths | example_interfaces/msg/Float64MultiArray |
| subscriber_node | 接收数组、计算均值、发布结果   | /mean_output   | example_interfaces/msg/Float64           |

循环输入三组数据：

1. 输入：`[10.0, 20.0, 30.0, 40.0]` → 均值 = (10+20+30+40)/4 = **25.00**，发布输出
2. 输入：`[]` 空数组 → 不计算、**不发布均值**，仅打印日志
3. 输入：`[5.0, 5.0, 5.0]` → 均值 = (5+5+5)/3 = **5.00**，发布输出

然后循环重复上面三组。

## 三、为什么空数组的均值不能直接返回 0

数学上，**空集合没有平均值**，求均值是总和除以元素个数，除以 0 属于非法运算。
如果直接返回 0，会产生歧义：

- 分不清：是数组本身数据的均值刚好等于 0，还是因为是空数组随便填的默认值；
所以代码使用 `std::optional<double>`，空数组返回 `std::nullopt`（代表 “无有效值”），而不是数字 0。

## 四、遇到的问题和解决方案

1. **问题：VSCode 里面 rclcpp、example_interfaces 头文件红色波浪线，但是 colcon build 编译正常**
原因：VSCode 的 IntelliSense 没有 ROS2 头文件搜索路径。
解决：配置 `.vscode/c_cpp_properties.json`，添加 ROS2 的 include 路径，或者启动 VSCode 前先 source ROS2 环境。
2. **问题：编译报错 undefined reference to meanLength**
原因：CMakeLists.txt 没有把 stats_lib 库链接给订阅节点。
解决：增加 `target_link_libraries(subscriber_node stats_lib)`。
3. **问题：ros2 run 提示找不到节点**
原因：CMake 没有写 install 安装规则，或者没有 source install/setup.bash。
解决：补充 install (TARGETS ...)，每次编译后 source 环境。
