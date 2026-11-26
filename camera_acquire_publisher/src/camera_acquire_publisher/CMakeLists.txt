cmake_minimum_required(VERSION 3.8)
project(camera_acquire_publisher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS2
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)

# 编译开关
option(USE_VIRTUAL "Use virtual camera" OFF)
option(USE_MINDVISION "Use MindVision camera" OFF)
option(USE_HIKROBOT "Use Hikrobot camera" OFF)

# 互斥检查
if( (USE_VIRTUAL AND USE_MINDVISION) OR (USE_VIRTUAL AND USE_HIKROBOT) OR (USE_MINDVISION AND USE_HIKROBOT) )
  message(FATAL_ERROR "Only one camera backend can be selected!")
endif()

set(SRC_LIST src/camera_publisher.cpp)
# ROS依赖，只放ROS包！OpenCV不放这里
set(DEPENDENCIES rclcpp sensor_msgs)
include_directories(include ${OpenCV_INCLUDE_DIRS})

if(USE_VIRTUAL)
  add_definitions(-DUSE_VIRTUAL)
  list(APPEND SRC_LIST src/virtual_grabber.cpp)
endif()

if(USE_MINDVISION)
  add_definitions(-DUSE_MINDVISION)
  set(MINDVISION_SDK_ROOT "/home/xiaoma/Downloads/mvsdk")
  include_directories(${MINDVISION_SDK_ROOT}/include)
  link_directories(${MINDVISION_SDK_ROOT}/lib/x64)
  list(APPEND SRC_LIST src/mindvision_grabber.cpp)
  list(APPEND LINK_LIBS MVSDK)
endif()

if(USE_HIKROBOT)
  add_definitions(-DUSE_HIKROBOT)
  set(HIK_SDK_ROOT "/opt/MVS")
  include_directories(${HIK_SDK_ROOT}/include)
  link_directories(${HIK_SDK_ROOT}/lib64)
  list(APPEND SRC_LIST src/hikrobot_grabber.cpp)
  list(APPEND LINK_LIBS MvCameraControl)
endif()

add_executable(${PROJECT_NAME}_node ${SRC_LIST})
ament_target_dependencies(${PROJECT_NAME}_node ${DEPENDENCIES})
# OpenCV在这里链接，不要进ament_target_dependencies
target_link_libraries(${PROJECT_NAME}_node ${LINK_LIBS} ${OpenCV_LIBS})

install(TARGETS
  ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
