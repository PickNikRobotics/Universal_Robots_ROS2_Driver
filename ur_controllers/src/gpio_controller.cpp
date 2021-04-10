// Copyright (c) 2021 PickNik LLC
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

#include <ur_controllers/gpio_controller.h>

#include "ur_controllers/gpio_controller.h"

namespace ur_controllers
{
controller_interface::InterfaceConfiguration ur_controllers::GPIOController::command_interface_configuration() const
{
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  for (size_t i = 0; i < 18; ++i)
  {
    config.names.emplace_back("gpio/standard_digital_output_cmd_" + std::to_string(i));
  }

  for (size_t i = 0; i < 2; ++i)
  {
    config.names.emplace_back("gpio/standard_analog_output_cmd_" + std::to_string(i));
  }

  config.names.emplace_back("gpio/io_async_success");

  config.names.emplace_back("gpio/speed_scaling_factor_cmd");

  config.names.emplace_back("gpio/scaling_async_success");

  return config;
}

controller_interface::InterfaceConfiguration ur_controllers::GPIOController::state_interface_configuration() const
{
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;

  // digital io
  for (size_t i = 0; i < 18; ++i)
  {
    config.names.emplace_back("gpio/digital_output_" + std::to_string(i));
  }

  for (size_t i = 0; i < 18; ++i)
  {
    config.names.emplace_back("gpio/digital_input_" + std::to_string(i));
  }

  // analog io
  for (size_t i = 0; i < 2; ++i)
  {
    config.names.emplace_back("gpio/standard_analog_output_" + std::to_string(i));
  }

  for (size_t i = 0; i < 2; ++i)
  {
    config.names.emplace_back("gpio/standard_analog_input_" + std::to_string(i));
  }

  for (size_t i = 0; i < 4; ++i)
  {
    config.names.emplace_back("gpio/analog_io_type_" + std::to_string(i));
  }

  // tool
  config.names.emplace_back("gpio/tool_mode");
  config.names.emplace_back("gpio/tool_output_voltage");
  config.names.emplace_back("gpio/tool_output_current");
  config.names.emplace_back("gpio/tool_temperature");

  for (size_t i = 0; i < 2; ++i)
  {
    config.names.emplace_back("gpio/tool_analog_input_" + std::to_string(i));
  }
  for (size_t i = 0; i < 2; ++i)
  {
    config.names.emplace_back("gpio/tool_analog_input_type_" + std::to_string(i));
  }

  // robot
  config.names.emplace_back("gpio/robot_mode");
  for (size_t i = 0; i < 4; ++i)
  {
    config.names.emplace_back("gpio/robot_status_bit_" + std::to_string(i));
  }

  // safety
  config.names.emplace_back("gpio/safety_mode");
  for (size_t i = 0; i < 11; ++i)
  {
    config.names.emplace_back("gpio/safety_status_bit_" + std::to_string(i));
  }

  return config;
}

controller_interface::return_type ur_controllers::GPIOController::init(const std::string& controller_name)
{
  initMsgs();

  return ControllerInterface::init(controller_name);
}

controller_interface::return_type ur_controllers::GPIOController::update()
{
  publishIO();
  publishToolData();
  publishRobotMode();
  publishSafetyMode();
  return controller_interface::return_type::SUCCESS;
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
ur_controllers::GPIOController::on_configure(const rclcpp_lifecycle::State& previous_state)
{
  try
  {
    // register publisher
    io_pub_ = get_node()->create_publisher<ur_msgs::msg::IOStates>("io_states", rclcpp::SystemDefaultsQoS());

    tool_data_pub_ = get_node()->create_publisher<ur_msgs::msg::ToolDataMsg>("tool_data", rclcpp::SystemDefaultsQoS());

    robot_mode_pub_ =
        get_node()->create_publisher<ur_dashboard_msgs::msg::RobotMode>("robot_mode", rclcpp::SystemDefaultsQoS());

    safety_mode_pub_ =
        get_node()->create_publisher<ur_dashboard_msgs::msg::SafetyMode>("safety_mode", rclcpp::SystemDefaultsQoS());

    set_io_srv_ = get_node()->create_service<ur_msgs::srv::SetIO>(
        "set_speed_slider", std::bind(&GPIOController::setIO, this, std::placeholders::_1, std::placeholders::_2));

    set_speed_slider_srv_ = get_node()->create_service<ur_msgs::srv::SetSpeedSliderFraction>(
        "set_io", std::bind(&GPIOController::setSpeedSlider, this, std::placeholders::_1, std::placeholders::_2));
  }
  catch (...)
  {
    return LifecycleNodeInterface::CallbackReturn::ERROR;
  }

  return LifecycleNodeInterface::on_configure(previous_state);
}

void GPIOController::publishIO()
{
  for (size_t i = 0; i < 18; ++i)
  {
    io_msg_.digital_out_states[i].state = static_cast<bool>(state_interfaces_[i].get_value());
    io_msg_.digital_in_states[i].state =
        static_cast<bool>(state_interfaces_[i + STATE_INTERFACES::DIGITAL_INPUTS].get_value());
  }

  for (size_t i = 0; i < 2; ++i)
  {
    io_msg_.analog_in_states[i].state =
        static_cast<float>(state_interfaces_[i + STATE_INTERFACES::ANALOG_INPUTS].get_value());
    io_msg_.analog_in_states[i].domain =
        static_cast<uint8_t>(state_interfaces_[i + STATE_INTERFACES::ANALOG_IO_TYPES].get_value());
  }

  for (size_t i = 0; i < 2; ++i)
  {
    io_msg_.analog_out_states[i].state =
        static_cast<float>(state_interfaces_[i + STATE_INTERFACES::ANALOG_OUTPUTS].get_value());
    io_msg_.analog_out_states[i].domain =
        static_cast<uint8_t>(state_interfaces_[i + STATE_INTERFACES::ANALOG_IO_TYPES + 2].get_value());
  }

  io_pub_->publish(io_msg_);
}

void GPIOController::publishToolData()
{
  tool_data_msg_.tool_mode = static_cast<uint8_t>(state_interfaces_[STATE_INTERFACES::TOOL_MODE].get_value());
  tool_data_msg_.analog_input_range2 =
      static_cast<uint8_t>(state_interfaces_[STATE_INTERFACES::TOOL_ANALOG_IO_TYPES].get_value());
  tool_data_msg_.analog_input_range3 =
      static_cast<uint8_t>(state_interfaces_[STATE_INTERFACES::TOOL_ANALOG_IO_TYPES + 1].get_value());
  tool_data_msg_.analog_input2 =
      static_cast<float>(state_interfaces_[STATE_INTERFACES::TOOL_ANALOG_INPUTS].get_value());
  tool_data_msg_.analog_input3 =
      static_cast<float>(state_interfaces_[STATE_INTERFACES::TOOL_ANALOG_INPUTS + 1].get_value());
  tool_data_msg_.tool_output_voltage =
      static_cast<uint8_t>(state_interfaces_[STATE_INTERFACES::TOOL_OUTPUT_VOLTAGE].get_value());
  tool_data_msg_.tool_current =
      static_cast<float>(state_interfaces_[STATE_INTERFACES::TOOL_OUTPUT_CURRENT].get_value());
  tool_data_msg_.tool_temperature =
      static_cast<float>(state_interfaces_[STATE_INTERFACES::TOOL_TEMPERATURE].get_value());
  tool_data_pub_->publish(tool_data_msg_);
}

void GPIOController::publishRobotMode()
{
  auto robot_mode = static_cast<int8_t>(state_interfaces_[STATE_INTERFACES::ROBOT_MODE].get_value());

  if (robot_mode_msg_.mode != robot_mode)
  {
    robot_mode_msg_.mode = robot_mode;
    robot_mode_pub_->publish(robot_mode_msg_);
  }
}

void GPIOController::publishSafetyMode()
{
  auto safety_mode = static_cast<uint8_t>(state_interfaces_[STATE_INTERFACES::SAFETY_MODE].get_value());

  if (safety_mode_msg_.mode != safety_mode)
  {
    safety_mode_msg_.mode = safety_mode;
    safety_mode_pub_->publish(safety_mode_msg_);
  }
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
ur_controllers::GPIOController::on_activate(const rclcpp_lifecycle::State& previous_state)
{
  return LifecycleNodeInterface::on_activate(previous_state);
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
ur_controllers::GPIOController::on_deactivate(const rclcpp_lifecycle::State& previous_state)
{
  return LifecycleNodeInterface::on_deactivate(previous_state);
}

bool GPIOController::setIO(ur_msgs::srv::SetIO::Request::SharedPtr req, ur_msgs::srv::SetIO::Response::SharedPtr resp)
{
  if (req->pin >= 0 && req->pin <= 17 && req->FUN_SET_DIGITAL_OUT)
  {
    // io async success
    command_interfaces_[COMMAND_INTERFACES::IO_ASYNC_SUCCESS].set_value(2.0);
    command_interfaces_[req->pin].set_value(static_cast<double>(req->state));

    while (command_interfaces_[COMMAND_INTERFACES::IO_ASYNC_SUCCESS].get_value() != 2.0)
    {
      // setting the value is not yet finished
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    resp->success = static_cast<bool>(command_interfaces_[20].get_value());
    return resp->success;
  }
  else if (req->pin >= 0 && req->pin <= 2 && req->FUN_SET_ANALOG_OUT)
  {
    // io async success
    command_interfaces_[COMMAND_INTERFACES::IO_ASYNC_SUCCESS].set_value(2.0);
    command_interfaces_[COMMAND_INTERFACES::ANALOG_OUTPUTS_CMD + req->pin].set_value(static_cast<double>(req->state));

    while (command_interfaces_[COMMAND_INTERFACES::IO_ASYNC_SUCCESS].get_value() != 2.0)
    {
      // setting the value is not yet finished
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    resp->success = static_cast<bool>(command_interfaces_[COMMAND_INTERFACES::IO_ASYNC_SUCCESS].get_value());
    return resp->success;
  }
  else
  {
    resp->success = false;
    return false;
  }
}

bool GPIOController::setSpeedSlider(ur_msgs::srv::SetSpeedSliderFraction::Request::SharedPtr req,
                                    ur_msgs::srv::SetSpeedSliderFraction::Response::SharedPtr resp)
{
  if (req->speed_slider_fraction >= 0.01 && req->speed_slider_fraction <= 1.0)
  {
    // reset success flag
    command_interfaces_[COMMAND_INTERFACES::SCALING_ASYNC_SUCCESS].set_value(2.0);
    // set commanding value for speed slider
    command_interfaces_[COMMAND_INTERFACES::SPEED_SCALING_CMD].set_value(
        static_cast<double>(req->speed_slider_fraction));

    while (command_interfaces_[COMMAND_INTERFACES::SCALING_ASYNC_SUCCESS].get_value() != 2.0)
    {
      // setting the value is not yet finished
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    resp->success = static_cast<bool>(command_interfaces_[COMMAND_INTERFACES::SCALING_ASYNC_SUCCESS].get_value());
  }
  else
  {
    resp->success = false;
    return false;
  }
  return true;
}

void GPIOController::initMsgs()
{
  io_msg_.digital_in_states.resize(standard_digital_output_cmd_.size());
  io_msg_.digital_out_states.resize(standard_digital_output_cmd_.size());
  io_msg_.analog_in_states.resize(2);
  io_msg_.analog_out_states.resize(2);
}

}  // namespace ur_controllers

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(ur_controllers::GPIOController, controller_interface::ControllerInterface)
