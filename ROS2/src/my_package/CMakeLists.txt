# 设置最低要求的CMake版本，ROS2 Humble常用3.8及以上，如果电脑CMake版本低于3.8,直接报错不能构建
cmake_minimum_required(VERSION 3.8)
# 定义包名：my_package和package.xml里的name必须保持一致
project(my_package)

# 判断编译器：是GCC或者Clang的话，增加编译警告选项
# -Wall：开启基础警告  -Wextra：开启额外警告  -Wpedantic：严格遵循C++标准，非标准写法会报警
# 作用：帮助提前发现代码隐患
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
# 查找ament_cmake：ROS2的构建系统核心包，ament就是ROS2的编译框架，必须要有，REQUIRED代表找不到就直接报错终止编译
find_package(ament_cmake REQUIRED)
# 查找rclcpp：ROS2 C++客户端库，节点，发布订阅，定时器，日志都依赖它
find_package(rclcpp REQUIRED)
# 查找example_interfaces，我们用到的Float64MultiArray，Float64消息就在这个包里
find_package(example_interfaces REQUIRED)

# BUILD_TESTING是CMake内置开关，默认打开，如果开启测试，才执行下面测试相关配置
# 平时我们colcon build不带测试，这一块基本不执行
if(BUILD_TESTING)
  # 自动代码检查工具包：ament_lint_auto,用来做代码格式，版权，cpplint静态检查
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  # 设置变量跳过版权检查
  # ROS2默认要求每个源码头部带版权声明，我们写练习代码没有版权注释，不跳过的话lint会报错，注释掉这行就会开启版权检查
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  # 跳过cpplint（C++代码规范检查），练习代码经常不符合谷歌编码规范，跳过避免一堆报错
  set(ament_cmake_cpplint_FOUND TRUE)
  # 自动寻找lint测试需要的依赖
  ament_lint_auto_find_test_dependencies()
endif()

# 生成 publisher_node 可执行程序，名字叫publisher_node，源码文件是src/publisher_node.cpp,就是前面的发布节点
add_executable(publisher_node src/publisher_node.cpp)
# ament专用命令：给publisher_node这个目标添加依赖rclcpp，example_interfaces
# 它会自动帮你处理头文件路径，链接库，ROS2 C++程序基本都用这个，代替原生cmake target_link_libraries
ament_target_dependencies(publisher_node rclcpp example_interfaces)

# 1. 把 8.1 的源码编译成一个静态库（体现题目要求）
# add_library(库名 源文件) 这里把src/stats.cpp编译成一个静态库，库名字叫stats_lib
# 对应stats.h：头文件放include/my_package/，实现写在stats.cpp,单独打包成库，而不是直接把代码塞到subscriber里面编译
add_library(stats_lib src/stats.cpp)
# 给stats_lib设置头文件搜索路径：
# PUBLIC：意味着链接这个库的目标（subscriber_node），自动继承这个头文件路径，不用再单独加include路径！
# $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>：编译阶段，头文件在包根目录的include文件夹
target_include_directories(stats_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  # $<INSTALL_INTERFACE:include>：安装之后，头文件在安装目录的include
  $<INSTALL_INTERFACE:include>
)
# 给stats_lib添加ament依赖，注释说了，我们的meanLength只是纯C++数字函数，其实不需要rclcpp,属于冗余写法，去掉也完全没问题
ament_target_dependencies(stats_lib rclcpp) # 虽然用不到，但加上无妨

# 2. 生成订阅者节点
# 编译订阅节点，可执行文件名subscriber_node，源码src/subscriber_node.cpp
add_executable(subscriber_node src/subscriber_node.cpp)
# 订阅节点依赖rclcpp和example_interfaces（消息）
ament_target_dependencies(subscriber_node rclcpp example_interfaces)

# 3. 关键：把刚才生成的库链接给订阅者节点！
# stats_lib 链接到subscriber_node,subscriber代码里调用meanLength()，这个函数定义在stats_lib库里
# 不链接这一行，编译会报undefined reference to meanLength未定义引用错误！
# 区分：ament_target_dependencies：ROS2系统包（rclcpp，msg包）
#      target_link_libraries：链接自己写的库/原生C++库
target_link_libraries(subscriber_node stats_lib)

# 安装规则（确保 ros2 run 能找到它们）
# 安装规则：把编译出来的两个可执行文件，安装到install/my_package/lib/my_package/
#         只有写了install，colcon build后，ros2 run my_package publisher_node，才能找到程序
#         不写install,只能在build目录手动运行
install(TARGETS
  publisher_node
  subscriber_node
  DESTINATION lib/${PROJECT_NAME}
)

# ament包收尾函数，必须放在文件最后一行，它会生成ament包的配置文件，让ament知道这个包的存在，依赖，导出信息，是ROS2 ament_cmake包的标志
ament_package()
