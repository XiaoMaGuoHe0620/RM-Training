// SPDX-License-Identifier: MIT
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

#include "camera_acquire_publisher/grabber.hpp"
#include "camera_acquire_publisher/virtual_grabber.hpp"
#if USE_HIKROBOT
#include "camera_acquire_publisher/hikrobot_grabber.hpp"
#endif
#if USE_MINDVISION
#include "camera_acquire_publisher/mindvision_grabber.hpp"
#endif

namespace camera_acquire {

// 根据编码计算每行字节数（step）
uint32_t encoding_step(const std::string& encoding, uint32_t width) {
  if (encoding == "rgb8" || encoding == "bgr8" || encoding == "rgba8" ||
      encoding == "bgra8") {
    return width * 3u;
  }
  if (encoding == "mono8" || encoding == "bayer_rg8" || encoding == "bayer_bg8" ||
      encoding == "bayer_gr8" || encoding == "bayer_gb8") {
    return width;
  }
  // 未知编码按单通道处理，避免越界
  return width;
}

class CameraPublishNode : public rclcpp::Node {
public:
  CameraPublishNode() : Node("camera_acquire_node") {
    // ---------------- 参数声明 ----------------
    declare_parameter<std::string>("camera_type", "virtual");
    declare_parameter<std::string>("topic_name", "camera/image_raw");
    declare_parameter<std::string>("frame_id", "camera");
    declare_parameter<std::string>("device_serial", "");
    declare_parameter<std::string>("timestamp_source", "system");  // system | hardware
    declare_parameter<int>("fps", 30);
    declare_parameter<int>("width", 640);
    declare_parameter<int>("height", 480);
    declare_parameter<std::string>("pixel_format", "rgb8");
    declare_parameter<bool>("qos_reliable", false);
    declare_parameter<int>("grab_timeout_ms", 1000);

    camera_type_    = get_parameter("camera_type").as_string();
    topic_name_     = get_parameter("topic_name").as_string();
    frame_id_       = get_parameter("frame_id").as_string();
    device_serial_  = get_parameter("device_serial").as_string();
    ts_source_      = get_parameter("timestamp_source").as_string();
    fps_            = get_parameter("fps").as_int();
    width_          = static_cast<uint32_t>(get_parameter("width").as_int());
    height_         = static_cast<uint32_t>(get_parameter("height").as_int());
    pixel_format_   = get_parameter("pixel_format").as_string();
    qos_reliable_   = get_parameter("qos_reliable").as_bool();
    timeout_ms_     = get_parameter("grab_timeout_ms").as_int();

    // ---------------- QoS：传感器数据默认 BestEffort + Volatile ----------------
    rclcpp::QoS qos = rclcpp::SensorDataQoS();
    qos.keep_last(5);
    if (qos_reliable_) {
      qos = rclcpp::QoS(rclcpp::KeepLast(5)).reliable().durability_volatile();
    }

    pub_ = create_publisher<sensor_msgs::msg::Image>(topic_name_, qos);

    // ---------------- 按类型创建取流后端 ----------------
    grabber_ = make_grabber(camera_type_);
    if (!grabber_) {
      RCLCPP_ERROR(get_logger(),
                   "不支持的 camera_type='%s'，或该后端未编译进节点。"
                   "可选：virtual（默认）；实机需在编译时加 -DUSE_HIKROBOT=ON / -DUSE_MINDVISION=ON",
                   camera_type_.c_str());
      throw std::runtime_error("unsupported camera_type");
    }

    if (!grabber_->open(device_serial_)) {
      RCLCPP_ERROR(get_logger(), "打开相机失败（type=%s, serial='%s'）",
                   camera_type_.c_str(), device_serial_.c_str());
      grabber_->close();
      throw std::runtime_error("camera open failed");
    }

    RCLCPP_INFO(get_logger(),
                "相机已打开：type=%s topic=%s frame_id=%s fps=%d qos=%s 时间戳来源=%s",
                camera_type_.c_str(), topic_name_.c_str(), frame_id_.c_str(), fps_,
                qos_reliable_ ? "RELIABLE" : "BEST_EFFORT", ts_source_.c_str());
  }

  ~CameraPublishNode() override {
    if (grabber_) {
      grabber_->close();
      RCLCPP_INFO(get_logger(), "相机已释放，节点退出");
    }
  }

  void run() {
    rclcpp::Rate rate(fps_);

    while (rclcpp::ok()) {
      RawFrame frame;
      if (!grabber_->grab(frame, timeout_ms_)) {
        RCLCPP_WARN(get_logger(), "取帧超时，继续重试（seq=%llu）",
                    static_cast<unsigned long long>(frame.frame_id_seq));
        rclcpp::spin_some(shared_from_this());
        rate.sleep();
        continue;
      }

      auto msg = std::make_unique<sensor_msgs::msg::Image>();
      msg->header.frame_id = frame_id_;

      // ---- 时间戳：system 用节点当前时间；hardware 用相机硬件时间 ----
      if (ts_source_ == "hardware" && frame.hardware_timestamp_ns > 0) {
        const auto ns = frame.hardware_timestamp_ns;
        msg->header.stamp.sec = static_cast<int32_t>(ns / 1'000'000'000ULL);
        msg->header.stamp.nanosec = static_cast<uint32_t>(ns % 1'000'000'000ULL);
      } else {
        msg->header.stamp = now();
      }

      msg->height = frame.height;
      msg->width = frame.width;
      msg->encoding = frame.encoding;
      msg->is_bigendian = 0;
      msg->step = encoding_step(frame.encoding, frame.width);
      msg->data = std::move(frame.data);

      pub_->publish(std::move(msg));
      rclcpp::spin_some(shared_from_this());
      rate.sleep();
    }
  }

private:
  std::unique_ptr<CameraGrabber> make_grabber(const std::string& type) {
    if (type == "virtual") {
      return std::make_unique<VirtualGrabber>(width_, height_, pixel_format_);
    }
#if USE_HIKROBOT
    if (type == "hikrobot") {
      return std::make_unique<HikRobotGrabber>();
    }
#endif
#if USE_MINDVISION
    if (type == "mindvision") {
      return std::make_unique<MindVisionGrabber>();
    }
#endif
    return nullptr;
  }

  std::string camera_type_, topic_name_, frame_id_, device_serial_, ts_source_,
      pixel_format_;
  int fps_, timeout_ms_;
  uint32_t width_, height_;
  bool qos_reliable_;

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_;
  std::unique_ptr<CameraGrabber> grabber_;
};

}  // namespace camera_acquire

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  auto node = std::make_shared<camera_acquire::CameraPublishNode>();
  node->run();
  rclcpp::shutdown();
  return 0;
}
