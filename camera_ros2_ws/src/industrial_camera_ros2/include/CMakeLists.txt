cmake_minimum_required(VERSION 3.16)
project(industrial_camera_ros2)

set(CMAKE_CXX_STANDARD 17)

find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)

# ---- 海康 MVS SDK（按实际安装路径修改）----
set(HIK_SDK_ROOT "/opt/MVS")   # Linux 常见路径，Windows下改为 C:/Program Files/MVS/...
find_library(HIK_LIB MvCameraControl HINTS ${HIK_SDK_ROOT}/lib)

# ---- 迈德威视 MVSDK（按实际安装路径修改）----
set(MVSDK_ROOT "/opt/MindVision/MVSDK")
find_library(MVSDK_LIB MVSDK HINTS ${MVSDK_ROOT}/lib)

add_executable(camera_node
  src/camera_node.cpp
  src/hik_camera.cpp
  src/mindvision_camera.cpp
)

target_include_directories(camera_node PRIVATE
  include
  ${HIK_SDK_ROOT}/include
  ${MVSDK_ROOT}/include
)

target_link_libraries(camera_node
  ${HIK_LIB}
  ${MVSDK_LIB}
)

ament_target_dependencies(camera_node rclcpp sensor_msgs cv_bridge)

install(TARGETS camera_node DESTINATION lib/${PROJECT_NAME})

ament_package()
