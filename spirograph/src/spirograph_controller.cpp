#include <cmath>
#include <functional>
#include <memory>
#include <vector>
#include <chrono>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "turtlesim/srv/set_pen.hpp"
#include "turtlesim/srv/teleport_absolute.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

class SpirographController : public rclcpp::Node
{
public:
  SpirographController()
  : Node("spirograph_controller"), phase(1), current_waypoint_index(0), pen_color(0), max_size(0)
  {
    generate_spirograph_waypoints();

    cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    pose_sub_ = this->create_subscription<turtlesim::msg::Pose>(
      "/turtle1/pose", 10, std::bind(&SpirographController::pose_callback, this, _1));

    pen_client_ = this->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen");
    teleport_client_ = this->create_client<turtlesim::srv::TeleportAbsolute>("/turtle1/teleport_absolute");
    
    RCLCPP_INFO(this->get_logger(), "Draw Spirograph");
  }

private:
  int phase;
  size_t current_waypoint_index;
  int pen_color;
  size_t max_size;
  std::pair<double, double> target;
  std::vector<std::pair<double, double>> waypoints1;
  std::vector<std::pair<double, double>> waypoints2;
  std::vector<std::pair<double, double>> waypoints3;
  
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
  rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;
  rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr pen_client_;
  rclcpp::Client<turtlesim::srv::TeleportAbsolute>::SharedPtr teleport_client_;

  void generate_spirograph_waypoints()
  {
    const double center_x = 5.5;
    const double center_y = 5.5;

    const double radius1 = 4.0;
    const int num_waypoints1 = 180;
    const int k1 = 6;
    const double angle = 0.35;

    for (int i = 0; i <= num_waypoints1; i++){
        double theta = 2.0 * M_PI * i / num_waypoints1;
        double r = radius1 * std::cos(k1 * theta);
        double x = center_x + r * std::cos(theta);
        double y = center_y + r * std::sin(theta);
        waypoints1.push_back({x, y});
    }

    for (int i = 0; i <= num_waypoints1; i++){
        double theta = 2.0 * M_PI * i / num_waypoints1;
        double r = radius1 * std::cos(k1 * theta);
        double x = r * std::cos(theta);
        double y = r * std::sin(theta);
        double x_now = x * std::cos(angle) - y * std::sin(angle);
        double y_now = x * std::sin(angle) + y * std::cos(angle);
        waypoints2.push_back({center_x + x_now, center_y + y_now});
    }

    const double radius2 = 2.5;
    const int num_waypoints2 = 180;
    int k2 = 12;

    for (int i = 0; i <= num_waypoints2; i++){
        double theta = 2.0 * M_PI * i / num_waypoints2;
        double r = radius2 * std::cos(k2 * theta);
        double x = center_x + r * std::cos(theta);
        double y = center_y + r * std::sin(theta);
        waypoints3.push_back({x, y});
    }
  }

  void change_color(int r, int g, int b, int width, int off){
    if(!pen_client_->wait_for_service(100ms)){
        return;
    }

    auto request = std::make_shared<turtlesim::srv::SetPen::Request>();
    request->r = r;
    request->g = g;
    request->b = b;
    request->width = width;
    request->off = off;

    pen_client_->async_send_request(request);
  }

  void teleport_turtle(double x, double y, double theta)
  {
    if(!teleport_client_->wait_for_service(100ms)){
        return;
    }

    auto request = std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
    request->x = x;
    request->y = y;
    request->theta = theta;

    teleport_client_->async_send_request(request);
  }

  void pose_callback(const turtlesim::msg::Pose & pose)
  {
    if (phase == 1) {
        max_size = waypoints1.size();
        if (current_waypoint_index >= max_size){
            phase = 2;
            current_waypoint_index = 0;
            change_color(255, 255, 255, 3, 1);
            teleport_turtle(waypoints2[0].first, waypoints2[0].second, 0.0);
            return;
        }
        target = waypoints1[current_waypoint_index];
    } else if (phase == 2) {
        max_size = waypoints2.size();
        if (current_waypoint_index >= max_size){
            phase = 3;
            current_waypoint_index = 0;
            change_color(255, 255, 255, 3, 1);
            teleport_turtle(waypoints3[0].first, waypoints3[0].second, 0.0);
            return;
        }
        target = waypoints2[current_waypoint_index];
    } else if (phase == 3) {
        max_size = waypoints3.size();
        if (current_waypoint_index >= max_size){
            geometry_msgs::msg::Twist cmd;
            cmd.linear.x = 0.0;
            cmd.angular.z = 0.0;
            cmd_pub_->publish(cmd);
            RCLCPP_INFO_ONCE(this->get_logger(), "Spirograph Complete");
            return;
        }
        target = waypoints3[current_waypoint_index];
    }

    if (current_waypoint_index == 0){
        change_color(255, 255, 255, 3, 1);
    } else if (current_waypoint_index == 1){
        if (phase == 1){
            change_color(50, 150, 255, 3, 0);
        } else if (phase == 2){
            change_color(138, 43, 226, 3, 0);
        } else if (phase == 3){
            change_color(0, 206, 209, 3, 0);
        }
    }

    if (phase != 3 && current_waypoint_index > 1 && current_waypoint_index % 25 == 0){
        pen_color = (pen_color + 1) % 3;
        if (pen_color == 0){
            change_color(50, 150, 255, 3, 0);
        } else if (pen_color == 1){
            change_color(138, 43, 226, 3, 0);
        } else if (pen_color == 2){
            change_color(186, 85, 211, 3, 0);
        }
    }

    const double dx = target.first - pose.x;
    const double dy = target.second - pose.y;
    const double distance = std::hypot(dx, dy);

    if (distance < 0.25){
        current_waypoint_index++;
        return;
    }

    geometry_msgs::msg::Twist cmd;
    double error = std::atan2(dy, dx) - pose.theta;
    error = std::atan2(std::sin(error), std::cos(error));

    cmd.angular.z = 6.0 * error;
    if (std::fabs(error) < 0.8) {
        cmd.linear.x = std::min(3.0 * distance, 3.0);
    } else {
        cmd.linear.x = 0.0;
    }
    cmd_pub_->publish(cmd);
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SpirographController>());
  rclcpp::shutdown();
  return 0;
}