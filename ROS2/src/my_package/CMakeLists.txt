cmake_minimum_required(VERSION 3.8)
project(my_package)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(example_interfaces REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# 生成 publisher_node 可执行文件
add_executable(publisher_node src/publisher_node.cpp)
ament_target_dependencies(publisher_node rclcpp example_interfaces)

# 1. 把 8.1 的源码编译成一个静态库（体现题目要求）
add_library(stats_lib src/stats.cpp)
target_include_directories(stats_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(stats_lib rclcpp) # 虽然用不到，但加上无妨

# 2. 生成订阅者节点
add_executable(subscriber_node src/subscriber_node.cpp)
ament_target_dependencies(subscriber_node rclcpp example_interfaces)

# 3. 关键：把刚才生成的库链接给订阅者节点！
target_link_libraries(subscriber_node stats_lib)

# 安装规则（确保 ros2 run 能找到它们）
install(TARGETS
  publisher_node
  subscriber_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
