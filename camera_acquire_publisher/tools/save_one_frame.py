#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""同帧留样工具：订阅一帧 sensor_msgs/Image，保存图片并打印作业要求的元信息。

用法：
  ros2 run camera_acquire_publisher save_one_frame.py
  ros2 run camera_acquire_publisher save_one_frame.py /camera/image_raw ./sample.png

依赖（桌面版 ROS 通常自带，缺失时）：
  sudo apt install python3-opencv ros-humble-cv-bridge
若无 OpenCV，脚本会自动退化为保存 PPM（纯 Python，无需第三方库）。
"""
import sys
import time

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image


class FrameSaver(Node):
    def __init__(self, topic: str, out_path: str):
        super().__init__("frame_saver")
        self.out_path = out_path
        self.saved = False
        self.sub = self.create_subscription(
            Image, topic, self.on_image, rclpy.qos.QoSPresetProfiles.SENSOR_DATA.value
        )

    def on_image(self, msg: Image):
        if self.saved:
            return
        self.saved = True

        stamp = msg.header.stamp
        sec = stamp.sec + stamp.nanosec / 1e9
        meta = {
            "话题名 (topic)": self.sub.topic_name,
            "分辨率 (width x height)": f"{msg.width} x {msg.height}",
            "发布编码 (encoding)": msg.encoding,
            "保存格式": "",
            "header.stamp": f"{stamp.sec}.{stamp.nanosec:09d}  ({sec:.6f}s)",
            "header.frame_id": msg.header.frame_id,
            "step": msg.step,
            "is_bigendian": msg.is_bigendian,
        }

        saved_path = self._save(msg)
        meta["保存格式"] = saved_path[0]
        self.get_logger().info("===== 同帧留样信息 =====")
        for k, v in meta.items():
            self.get_logger().info(f"{k}: {v}")
        self.get_logger().info(f"已保存: {saved_path[1]}")
        raise SystemExit(0)

    def _save(self, msg: Image):
        # 优先用 cv_bridge + OpenCV 存 PNG
        try:
            import cv2  # type: ignore
            from cv_bridge import CvBridge  # type: ignore

            bridge = CvBridge()
            cv_img = bridge.imgmsg_to_cv2(msg, desired_encoding="rgb8")
            # imgmsg_to_cv2 得到 RGB，OpenCV 存图需要 BGR
            if msg.encoding in ("rgb8", "bayer_rg8", "bayer_bg8",
                                 "bayer_gr8", "bayer_gb8"):
                cv_img = cv2.cvtColor(cv_img, cv2.COLOR_RGB2BGR)
            cv2.imwrite(self.out_path, cv_img)
            return "PNG", self.out_path
        except Exception as e:  # 退化：纯 Python 存 PPM
            self.get_logger().warn(f"OpenCV 不可用，改存 PPM（原因: {e}）")
            ppm_path = self.out_path.rsplit(".", 1)[0] + ".ppm"
            maxval = 255
            channels = 3 if msg.encoding in ("rgb8", "bgr8") else 1
            header = (
                f"P6\n{msg.width} {msg.height}\n{maxval}\n"
                if channels == 3
                else f"P5\n{msg.width} {msg.height}\n{maxval}\n"
            )
            with open(ppm_path, "wb") as f:
                f.write(header.encode())
                f.write(bytes(msg.data))
            return "PPM", ppm_path


def main():
    rclpy.init()
    topic = sys.argv[1] if len(sys.argv) > 1 else "/camera/image_raw"
    out_path = sys.argv[2] if len(sys.argv) > 2 else "frame_sample.png"
    node = FrameSaver(topic, out_path)
    try:
        rclpy.spin(node)
    except SystemExit:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
