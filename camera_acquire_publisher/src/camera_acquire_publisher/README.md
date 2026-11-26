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
cd ~/my-notes/camera_acquire_publisher
rm -rf build install log
colcon build --cmake-args -DUSE_MINDVISION=ON
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

### 1.3 海康相机
```bash
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
source ~/my-notes/camera_acquire_publisher/install/setup.bash
ros2 topic list
ros2 topic hz /camera/image_raw
```

---