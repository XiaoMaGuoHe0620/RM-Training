# camera_acquire_publisher
一套代码，用编译开关切换三种后端：虚拟相机、迈德威视、海康机器人。

## 三个后端（互斥，每次切换先清缓存）

| 后端 | 编译开关 | 硬件 |
|---|---|---|
| 虚拟相机 | `-DUSE_VIRTUAL=ON` | 不需要相机 |
| 迈德威视 | `-DUSE_MINDVISION=ON` | USB3 相机 |
| 海康机器人 | `-DUSE_HIKROBOT=ON` | GigE 网口相机 |

> 编译开关在编译期固定，运行时改不了。换后端必须先清缓存再重新编译。

## 1. 编译

### 1.1 虚拟相机 
```bash
cd ~/my-notes/camera_acquire_publisher
rm -rf build install log
colcon build --cmake-args -DUSE_VIRTUAL=ON
```

#### 终端A
```bash
cd ~/my-notes/camera_acquire_publisher
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run camera_acquire_publisher camera_acquire_publisher_node
```

#### 终端B
```bash
source /opt/ros/humble/setup.bash
source ~/my-notes/camera_acquire_publisher/install/setup.bash
ros2 topic list
```

### 1.2 迈德相机
```bash
cd ~/Downloads/mvsdk
sudo ./install.sh
sudo udevadm control --reload-rules
sudo udevadm trigger

cd ~/my-notes/camera_acquire_publisher
rm -rf build install log
colcon build --cmake-args -DUSE_MINDVISION=ON

source /opt/ros/humble/setup.bash
source install/setup.bash
```

#### 终端A
```bash
cd ~/my-notes/camera_acquire_publisher
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run camera_acquire_publisher camera_acquire_publisher_node
```

#### 终端B
```bash
source /opt/ros/humble/setup.bash
ros2 run image_view image_view --ros-args -r image:=/camera/image_raw
```

### 1.3 海康相机
```bash
ls /opt/MVS/include/MvCameraControl.h
ls /opt/MVS/lib/64/libMvCameraControl.so

source /opt/ros/humble/setup.bash
export LD_LIBRARY_PATH=/opt/MVS/lib/64:$LD_LIBRARY_PATH

ip addr

cd ~/my-notes/camera_acquire_publisher
rm -rf build install log
colcon build --cmake-args -DUSE_HIKROBOT=ON
```

#### 终端A
```bash
cd ~/my-notes/camera_acquire_publisher
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 run camera_acquire_publisher camera_acquire_publisher_node
```

#### 终端B
```bash
source /opt/ros/humble/setup.bash
ros2 run image_view image_view --ros-args -r image:=/camera/image_raw
```

---

## 2.发布信息

| 项目 | 值 |
|---|---|
| 话题 | `/camera/image_raw` |
| 消息类型 | `sensor_msgs/msg/Image` |
| 编码 | bgr8（彩色） |
| QoS | SensorDataQoS（BestEffort） |
| frame_id | `camera` |

---

## 3.常见问题

- **报 `No xxx camera found`**：没插相机、没供电、权限没装，或编译的后端和硬件对不上。

---
