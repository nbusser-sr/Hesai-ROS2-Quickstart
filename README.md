# Hesai ROS2 Jazzy quickstart

## Config

Modify the following parameters in `src/HesaiLidar_ROS_2.0/config/config.yaml`:
- `udp_port`
- `device_ip_address`
- `correction_file_path`
- `firetimes_path`

## Build

```sh
source /opt/ros/jazzy/setup.zsh
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Debug
```

## Run

```sh
source install/local_setup.zsh
ros2 launch hesai_ros_driver start.py
```

## LLDB

```sh
lldb -f build/hesai_ros_driver/hesai_ros_driver_node
settings set target.source-map /workspace/ros2_ws/src/HesaiLidar_ROS_2.0 /workspace/ros2_ws/build/hesai_ros_driver/src/driver/HesaiLidar_SDK_2.0
breakpoint set --file src/driver/HesaiLidar_SDK_2.0/driver/hesai_lidar_sdk.hpp --line 171
```
