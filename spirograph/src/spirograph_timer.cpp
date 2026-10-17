#include <chrono>
#include <functional>
#include <memory>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class SpirographTimer : public rclcpp::Node
{
public:
  SpirographTimer()
  : Node("spirograph_timer")
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    timer_ = this->create_wall_timer(
      100ms, std::bind(&SpirographTimer::timer_callback, this));
  }

private:
  void timer_callback()
  {
    geometry_msgs::msg::Twist msg;
    msg.linear.x = 1.5;
    msg.angular.z = 2.5;
    publisher_->publish(msg);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SpirographTimer>());
  rclcpp::shutdown();
  return 0;
}