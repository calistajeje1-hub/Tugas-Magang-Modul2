cmake_minimum_required(VERSION 3.5)
project(spirograph)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)

add_executable(spirograph_timer src/spirograph_timer.cpp)
ament_target_dependencies(spirograph_timer rclcpp geometry_msgs)

add_executable(spirograph_listener src/spirograph_listener.cpp)
ament_target_dependencies(spirograph_listener rclcpp turtlesim)

add_executable(spirograph_controller src/spirograph_controller.cpp)
ament_target_dependencies(spirograph_controller rclcpp geometry_msgs turtlesim)

install(TARGETS
  spirograph_timer
  spirograph_listener
  spirograph_controller
  DESTINATION lib/${PROJECT_NAME})

ament_package()
