// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cinttypes>
#include <memory>
#include "rclcpp/parameter.hpp"
#include "rclcpp/rclcpp.hpp"

void do_test(int argc, char ** argv, bool declare_before_subscribe=true) {
  rclcpp::init(argc, argv);
  static const char * param_name = "timing_test_param";
  rclcpp::Node::SharedPtr node = std::make_shared<rclcpp::Node>("my_node");
  auto start = std::chrono::system_clock::now();

  if (declare_before_subscribe) {
    node->declare_parameter(param_name, rclcpp::ParameterValue(false));
  }
  auto client = std::make_shared<rclcpp::AsyncParametersClient>(node);

  auto sub = client->on_parameter_event([start](const rcl_interfaces::msg::ParameterEvent::SharedPtr event) -> void {
    (void) event;
    auto now = std::chrono::system_clock::now();
    auto diff = now - start;
    RCUTILS_LOG_INFO_NAMED("minimal_param", "Got event %ld ms after set", std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
  });

  start = std::chrono::system_clock::now();

  if (!declare_before_subscribe) {
    node->declare_parameter(param_name, rclcpp::ParameterValue(false));
  }

  client->set_parameters({
    rclcpp::Parameter(param_name, rclcpp::ParameterValue(true))
  });

  rclcpp::spin(node);
  rclcpp::shutdown();
}


int main(int argc, char * argv[])
{
  // Setting up subscriber before any publishes, I get 2 "Got event" callbacks at about 80ms
  RCUTILS_LOG_INFO_NAMED("minimal_param", "Running declare _after_ subscribe");
  do_test(argc, argv, false);
  // Setting up subscriber after declaration publish, I get 1 "Got event" callback at about 3000ms
  RCUTILS_LOG_INFO_NAMED("minimal_param", "Running declare _before_ subscribe");
  do_test(argc, argv, true);
  return 0;
}
