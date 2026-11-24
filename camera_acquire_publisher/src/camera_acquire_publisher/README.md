# camera_acquire_publisher —— ROS 2 C++ 相机取流与发布节点

> 作业：用 C++ 自写 ROS 2 相机节点，从工业相机连续取帧，以 `sensor_msgs/msg/Image`
> 发布未标注原图；兼容海康机器人与迈德威视两种工业相机。
> 本工程默认内置**虚拟相机**，在没有实机、没有任何厂商 SDK 的情况下即可编译运行并完成全部验收；
> 装上厂商 SDK 后，改一个编译选项和一个参数即可切到实机。

---

## 0. 作业要求对照

| 作业要求 | 本工程对应实现 |
|---|---|
| C++ 编写 ROS 2 节点，连续取帧，发布 `sensor_msgs/msg/Image` 未标注原图 | `src/main.cpp` + `src/*_grabber.cpp`，发布 `/camera/image_raw` |
| 实机验收：画面变化图像随之更新 | 虚拟相机每帧生成运动白块+渐变；实机走 SDK 取帧循环 |
| 节点正常退出释放相机 | `close()` 停止取流、关设备、销毁句柄；`main.cpp` 析构兜底 |
| 按 README 重启后恢复取流 | 每次 `open()` 重新枚举/打开，无残留句柄 |
| 同帧留样：原图 + 话题名、分辨率、发布编码、保存格式、`header.stamp`、`header.frame_id` | `tools/save_one_frame.py` 存 PNG 并打印全部元信息 |
| 工程材料：源码、构建文件、README，不含构建产物和日志 | 只提交 `src/ include/ launch/ config/ tools/ package.xml CMakeLists.txt README.md` |
| README 写明相机型号、采集接口与依赖、构建及启动方式、发布话题与 QoS、时间戳来源 | 见本文第 1、6、7 节 |
| 多型号兼容：海康 + 迈德威视 | `hikrobot_grabber.cpp`（MVS）、`mindvision_grabber.cpp`（MVSDK），CMake 选项分别开启 |

---

## 1. 环境（Ubuntu 22.04 + ROS 2 Humble）

### 1.1 安装 ROS 2 Humble（已装可跳过）

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo apt update && sudo apt install -y curl gnupg lsb-release

sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
  http://packages.ros.org/ros2/ubuntu $(lsb_release -cs) main" \
  | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

sudo apt update
sudo apt install -y ros-humble-desktop ros-dev-tools python3-colcon-common-extensions
```

把环境变量写进 `~/.bashrc`（每次开终端自动 source）：

```bash
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

验证：

```bash
ros2 --help && echo OK
```

### 1.2 装可视化与留样依赖

```bash
sudo apt install -y ros-humble-rqt-image-view ros-humble-cv-bridge python3-opencv
```

---

## 2. 工作区与目录结构

```
camera_ws/
└── src/
    └── camera_acquire_publisher/
        ├── package.xml
        ├── CMakeLists.txt
        ├── README.md
        ├── config/
        │   └── camera_params.yaml        # 默认参数（改这里即可，不动代码）
        ├── launch/
        │   └── camera.launch.py          # 启动文件，可传 camera_type
        ├── include/camera_acquire_publisher/
        │   ├── grabber.hpp               # 取流后端统一接口
        │   ├── virtual_grabber.hpp
        │   ├── hikrobot_grabber.hpp
        │   └── mindvision_grabber.hpp
        ├── src/
        │   ├── main.cpp                  # 节点：参数、QoS、打戳、发布、退出释放
        │   ├── virtual_grabber.cpp       # 虚拟相机（默认编译，无依赖）
        │   ├── hikrobot_grabber.cpp      # 海康 MVS（-DUSE_HIKROBOT=ON）
        │   └── mindvision_grabber.cpp    # 迈德威视 MVSDK（-DUSE_MINDVISION=ON）
        └── tools/
            └── save_one_frame.py         # 同帧留样工具
```

建工作区并把本包放进 `camera_ws/src/`：

```bash
mkdir -p ~/camera_ws/src
cd ~/camera_ws/src
# 把 camera_acquire_publisher 整个文件夹复制到这里
```

---

## 3. 编译（默认：虚拟相机，无需 SDK）

```bash
cd ~/camera_ws
colcon build --packages-select camera_acquire_publisher
source install/setup.bash
```

> 默认只编译 `main.cpp + virtual_grabber.cpp`，海康/迈德威视源码在目录里但不参与编译，
> 所以没有 SDK 也能 100% 编译通过。

---

## 4. 虚拟测试（无实机、无 SDK 时完成验收）

### 4.1 启动节点

```bash
ros2 launch camera_acquire_publisher camera.launch.py
# 或指定后端：ros2 launch camera_acquire_publisher camera.launch.py camera_type:=virtual
```

另开一个终端（记得 `source ~/camera_ws/install/setup.bash`）：

### 4.2 验证①：画面变化时图像随之更新

```bash
# 看图像（运动白块应持续移动）
ros2 run rqt_image_view rqt_image_view
# 左上角选择 /camera/image_raw

# 看帧率，应稳定在 ~30
ros2 topic hz /camera/image_raw

# 看一帧消息结构（宽度/高度/编码/stamp/frame_id）
ros2 topic echo /camera/image_raw --once
```

### 4.3 验证②：正常退出释放相机

在启动节点的终端按 `Ctrl+C`，日志应打印：

```
相机已释放，节点退出
```

进程应完全退出（`ps aux | grep camera_acquire_node` 无残留）。

### 4.4 验证③：重启后恢复取流

再次 `ros2 launch ...`，`ros2 topic hz /camera/image_raw` 应立刻恢复 ~30 Hz，
不报错、不报“设备被占用”。

---

## 5. 同帧留样

节点运行中，另开终端：

```bash
ros2 run camera_acquire_publisher save_one_frame.py /camera/image_raw ./frame_sample.png
```

脚本保存一张 `frame_sample.png`，并打印作业要求的全部信息，示例：

```
话题名 (topic): /camera/image_raw
分辨率 (width x height): 640 x 480
发布编码 (encoding): rgb8
保存格式: PNG
header.stamp: 1763956812.034567890  (1763956812.034568s)
header.frame_id: camera
step: 1920
is_bigendian: 0
```

把这张 PNG + 上面这张信息表一起提交即可。

---

## 6. 实机接入（有相机后再做）

### 6.1 海康机器人（Hikrobot）MVS

1. 到海康机器人官网下载对应系统的 MVS 安装包（`MVS_x.x.x_xxx_ubuntu22.04_amd64.deb`）：
   ```bash
   sudo dpkg -i MVS_*.deb
   # 安装完成后默认在 /opt/MVS（头文件 /opt/MVS/include，库 /opt/MVS/lib/64）
   ```
2. 先运行厂商客户端 `MVS`（菜单里或终端 `MVS`）确认能枚举到相机、能出图；
   记下相机序列号（SerialNumber）。
3. 带选项重新编译：
   ```bash
   cd ~/camera_ws
   colcon build --packages-select camera_acquire_publisher \
     --cmake-args -DUSE_HIKROBOT=ON
   # SDK 不在 /opt/MVS 时加 -DHIKROBOT_DIR=/你的/MVS路径
   source install/setup.bash
   ```
4. 改 `config/camera_params.yaml`：
   ```yaml
   camera_type: "hikrobot"
   device_serial: "相机序列号"
   ```
5. 启动：
   ```bash
   ros2 launch camera_acquire_publisher camera.launch.py
   ```

### 6.2 迈德威视（MindVision）MVSDK

1. 官网下载 Linux x64 MVSDK，按官方说明安装（默认 `/opt/MindVision`，头文件 `MVSDK.h`）。
2. 先用厂商 Demo 确认枚举、出图正常，记下序列号。
3. 带选项重新编译：
   ```bash
   cd ~/camera_ws
   colcon build --packages-select camera_acquire_publisher \
     --cmake-args -DUSE_MINDVISION=ON
   # SDK 不在 /opt/MindVision 时加 -DMINDVISION_DIR=/你的/MVSDK路径
   source install/setup.bash
   ```
4. 改 `config/camera_params.yaml`：
   ```yaml
   camera_type: "mindvision"
   device_serial: "相机序列号"
   ```
5. 启动：
   ```bash
   ros2 launch camera_acquire_publisher camera.launch.py
   ```

> 注意：迈德威视新旧版 SDK 函数名有 `MV_CC_*`（新）与 `MV_*`（旧）两套，
> `src/mindvision_grabber.cpp` 按新版 MVSDK 编写；安装后对照 `MVSDK.h` 核对函数名即可，
> 取流流程（枚举→打开→连续取流→取帧→Bayer 转 RGB8→释放）不变。海康同理，
> 以 `/opt/MVS/include/MvCameraControl.h` 为准。

### 6.3 相机型号填写（README 提交项）

提交时在本节补上实机信息（示例）：

- 相机型号：海康机器人 MV-CA060-10GC / 迈德威视 XXX
- 采集接口：GigE 或 USB3 Vision
- 依赖：MVS SDK x.x.x / MVSDK x.x.x；rclcpp、sensor_msgs（Humble）

---

## 7. 发布话题、QoS 与时间戳来源

| 项目 | 说明 |
|---|---|
| 话题名 | `/camera/image_raw`（参数 `topic_name` 可改） |
| 消息类型 | `sensor_msgs/msg/Image` |
| 发布编码 | 虚拟 `rgb8`；实机单色透传 `mono8`，Bayer 经 SDK 转 `rgb8` |
| QoS | 默认 `SensorDataQoS`：BestEffort + Volatile，KeepLast(5)；`qos_reliable: true` 切 Reliable |
| 时间戳来源 | `timestamp_source: system`（默认）= 节点收到帧时 `now()` 打系统时间；`hardware` = 用 SDK 返回的相机硬件时间戳（ns）填 `header.stamp` |
| frame_id | `camera`（参数 `frame_id` 可改） |

---

## 8. 提交清单（不要带构建产物和日志）

只打包这些：

```
camera_acquire_publisher/
├── package.xml
├── CMakeLists.txt
├── README.md
├── config/camera_params.yaml
├── launch/camera.launch.py
├── include/camera_acquire_publisher/*.hpp
├── src/*.cpp
└── tools/save_one_frame.py
```

不要提交：`build/`、`install/`、`log/`、`__pycache__/`、`*.png` 调试中间文件（留样图单独交）。

---

## 9. 常见问题

- **`colcon build` 报找不到 MvCameraControl.h / libMVSDK.so**：说明开了后端选项但没装 SDK；
  去掉 `-DUSE_HIKROBOT=ON` / `-DUSE_MINDVISION=ON` 回到虚拟编译即可。
- **`rqt_image_view` 看不到图**：确认 `source install/setup.bash`；`ros2 topic list` 有
  `/camera/image_raw`，`ros2 topic hz` 有频率；QoS 不匹配时 rqt 一般自动兼容。
- **实机报设备被占用**：上一个节点没退出干净，`ps aux | grep camera` 杀掉残留进程，
  或拔插相机后重启；`close()` 已保证正常退出释放。
- **GigE 相机枚举不到**：检查网线、IP 是否与电脑同网段，关闭防火墙，
  厂商客户端能看到再跑本节点。
