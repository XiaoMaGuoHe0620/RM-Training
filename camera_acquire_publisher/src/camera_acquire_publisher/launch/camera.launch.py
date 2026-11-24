# SPDX-License-Identifier: MIT
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory("camera_acquire_publisher")
    default_params = os.path.join(pkg_share, "config", "camera_params.yaml")

    camera_type_arg = DeclareLaunchArgument(
        "camera_type",
        default_value="virtual",
        description="取流后端：virtual | hikrobot | mindvision（后两者需编译时开启）",
    )

    camera_node = Node(
        package="camera_acquire_publisher",
        executable="camera_acquire_node",
        name="camera_acquire_node",
        output="screen",
        parameters=[
            default_params,
            {"camera_type": LaunchConfiguration("camera_type")},
        ],
    )

    return LaunchDescription([camera_type_arg, camera_node])
