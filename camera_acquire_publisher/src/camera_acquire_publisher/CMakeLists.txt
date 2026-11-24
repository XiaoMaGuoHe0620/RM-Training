cmake_minimum_required(VERSION 3.8)
project(camera_acquire_publisher)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# 可选后端开关（默认全部关闭 => 只编译虚拟相机，无需任何厂商 SDK）
#   colcon build --cmake-args -DUSE_HIKROBOT=ON
#   colcon build --cmake-args -DUSE_MINDVISION=ON
# ---------------------------------------------------------------------------
option(USE_HIKROBOT    "Enable Hikrobot MVS SDK backend"   OFF)
option(USE_MINDVISION  "Enable MindVision MVSDK backend"    OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

# ---------------------------------------------------------------------------
# 公共源文件：节点主程序 + 虚拟相机（无 SDK 也能跑）
# ---------------------------------------------------------------------------
set(CORE_SOURCES
  src/main.cpp
  src/virtual_grabber.cpp
)

# ---------------------------------------------------------------------------
# 海康机器人 MVS SDK 后端
#   SDK 默认安装路径：/opt/MVS （头文件 /opt/MVS/include，库 /opt/MVS/lib/64）
#   也可通过 -DHIKROBOT_DIR=/path/to/MVS 指定
# ---------------------------------------------------------------------------
if(USE_HIKROBOT)
  if(NOT DEFINED HIKROBOT_DIR)
    set(HIKROBOT_DIR "/opt/MVS" CACHE PATH "Hikrobot MVS SDK root")
  endif()
  find_library(MVS_LIB
    NAMES MvCameraControl mvCameraControl
    PATHS "${HIKROBOT_DIR}/lib/64" "${HIKROBOT_DIR}/lib" /usr/lib /usr/local/lib
    REQUIRED)
  find_path(MVS_INCLUDE
    NAMES MvCameraControl.h
    PATHS "${HIKROBOT_DIR}/include" /usr/include /usr/local/include
    REQUIRED)
  message(STATUS "[Hikrobot] SDK: ${MVS_LIB} / ${MVS_INCLUDE}")
  list(APPEND CORE_SOURCES src/hikrobot_grabber.cpp)
  add_definitions(-DUSE_HIKROBOT=1)
endif()

# ---------------------------------------------------------------------------
# 迈德威视 MVSDK 后端
#   SDK 默认安装路径：/opt/MindVision 或 /usr/local/MVSDK
#   也可通过 -DMINDVISION_DIR=/path/to/MVSDK 指定
# ---------------------------------------------------------------------------
if(USE_MINDVISION)
  if(NOT DEFINED MINDVISION_DIR)
    set(MINDVISION_DIR "/opt/MindVision" CACHE PATH "MindVision MVSDK root")
  endif()
  find_library(MVSDK_LIB
    NAMES MVSDK mvSDK MVSDK_C
    PATHS "${MINDVISION_DIR}/lib" "${MINDVISION_DIR}/lib/x64" /usr/lib /usr/local/lib
    REQUIRED)
  find_path(MVSDK_INCLUDE
    NAMES MVSDK.h
    PATHS "${MINDVISION_DIR}/include" /usr/include /usr/local/include
    REQUIRED)
  message(STATUS "[MindVision] SDK: ${MVSDK_LIB} / ${MVSDK_INCLUDE}")
  list(APPEND CORE_SOURCES src/mindvision_grabber.cpp)
  add_definitions(-DUSE_MINDVISION=1)
endif()

# ---------------------------------------------------------------------------
# 可执行节点
# ---------------------------------------------------------------------------
add_executable(camera_acquire_node ${CORE_SOURCES})
target_include_directories(camera_acquire_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(camera_acquire_node rclcpp::rclcpp)
ament_target_dependencies(camera_acquire_node rclcpp sensor_msgs)

if(USE_HIKROBOT)
  target_include_directories(camera_acquire_node PRIVATE ${MVS_INCLUDE})
  target_link_libraries(camera_acquire_node ${MVS_LIB})
endif()
if(USE_MINDVISION)
  target_include_directories(camera_acquire_node PRIVATE ${MVSDK_INCLUDE})
  target_link_libraries(camera_acquire_node ${MVSDK_LIB})
endif()

install(TARGETS camera_acquire_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME})

install(PROGRAMS tools/save_one_frame.py
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
