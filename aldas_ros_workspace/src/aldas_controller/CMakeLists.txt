cmake_minimum_required(VERSION 3.8)
project(aldas_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(px4_msgs REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# =================================================================
# ALDAS CONTROLLER TARGETS (Added code)
# =================================================================

# 1. Tell CMake to compile your new node
add_executable(offboard_hover_node src/offboard_hover_node.cpp)

# 2. Link the executable to the required ROS 2 and PX4 message packages
ament_target_dependencies(offboard_hover_node rclcpp geometry_msgs px4_msgs)

# 3. Ensure the binary is installed to the correct ROS 2 directory structure
install(TARGETS
  offboard_hover_node
  DESTINATION lib/${PROJECT_NAME}
)

# =================================================================
# This must ALWAYS remain the last line!
ament_package()