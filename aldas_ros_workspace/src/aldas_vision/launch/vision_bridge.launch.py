from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # Define the Gazebo topic names
    gz_image_topic = '/world/default/model/x500_docking_0/link/lidar_sensor_link/sensor/downward_camera/image'
    gz_info_topic = '/world/default/model/x500_docking_0/link/lidar_sensor_link/sensor/downward_camera/camera_info'

    # The bridge node that translates Gazebo data to ROS 2
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='camera_bridge',
        arguments=[
            f'{gz_image_topic}@sensor_msgs/msg/Image[gz.msgs.Image',
            f'{gz_info_topic}@sensor_msgs/msg/CameraInfo[gz.msgs.CameraInfo'
        ],
        remappings=[
            (gz_image_topic, '/camera/image_raw'),
            (gz_info_topic, '/camera/camera_info'),
        ],
        output='screen'
    )

    # Your custom vision node
    vision_node = Node(
        package='aldas_vision',
        executable='aldas_vision_node',
        name='aldas_vision_node',
        output='screen'
    )

    return LaunchDescription([
        bridge_node,
        vision_node
    ])