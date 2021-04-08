// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// Copyright 2019 FZI Forschungszentrum Informatik
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
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Felix Exner exner@fzi.de
 * \date    2019-11-04
 *
 */
//----------------------------------------------------------------------
#pragma once

// ROS
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/create_server.hpp>
#include <std_srvs/srv/trigger.h>

#include <sstream>
#include <ur_client_library/ur/datatypes.h>
#include <ur_dashboard_msgs/msg/robot_mode.hpp>
#include <ur_dashboard_msgs/msg/safety_mode.hpp>
#include <ur_dashboard_msgs/action/set_mode.hpp>
#include <std_srvs/srv/trigger.hpp>

namespace ur_robot_driver
{
/*!
 * \brief A small helper class around the robot state (constisting of 'robot_mode' and
 * 'safety_mode')
 *
 * This class logs any changes of the robot state and provides an action interface bring the robot
 * into a required state. For modifying the robot mode it uses the dashboard server interface
 * provided by the ur_robot_driver.
 */
class RobotStateHelper
{
public:
  using SetModeGoalHandle = rclcpp_action::ServerGoalHandle<ur_dashboard_msgs::action::SetMode>;
  /*!
   * \brief Constructor that should be used by default
   *
   * \param nh Node handle that should be used. The handle's namespace should be the same as used
   * for the hardware interface. Otherwise remapping will be necessary to access the hardware
   * interface's topics and sercices.
   */
  RobotStateHelper(rclcpp::Node::SharedPtr node);
  RobotStateHelper() = delete;
  virtual ~RobotStateHelper() = default;

private:
  rclcpp::Node::SharedPtr node_;

  void robotModeCallback(ur_dashboard_msgs::msg::RobotMode::SharedPtr msg);
  void safetyModeCallback(ur_dashboard_msgs::msg::SafetyMode::SharedPtr msg);

  /*!
   * \brief Updates action feedback and triggers next transition if necessary
   */
  void updateRobotState();

  /*!
   * \brief Performs the transition required by the current mode to get to the next mode.
   */
  void doTransition();

  /*!
   * \brief Small wrapper function to call a trigger service. The trigger's response message will be
   * loged to INFO output.
   *
   * \param srv Pointer to service client that shall be used.
   *
   * \returns service response's success field.
   */
  bool safeDashboardTrigger(rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr srv);

  // Action server stuff

  void setModeAcceptCallback(std::shared_ptr<SetModeGoalHandle> goal_handle);

  rclcpp_action::GoalResponse setModeGoalCallback(const rclcpp_action::GoalUUID& uuid,
                                                  std::shared_ptr<const ur_dashboard_msgs::action::SetMode::Goal> goal);

  rclcpp_action::CancelResponse
  setModeCancelCallback(std::shared_ptr<const RobotStateHelper::SetModeGoalHandle> goal_handle);

//  void setModeGoalCallbackOld();
//  void setModePreemptCallbackOld();
  void startActionServer();
  bool is_started_;

  ur_dashboard_msgs::action::SetMode::Goal::ConstSharedPtr goal_;
  ur_dashboard_msgs::action::SetMode::Feedback feedback_;
  ur_dashboard_msgs::action::SetMode::Result result_;

  rclcpp_action::Server<ur_dashboard_msgs::action::SetMode>::SharedPtr set_mode_as_;

  urcl::RobotMode robot_mode_;
  urcl::SafetyMode safety_mode_;

  rclcpp::Subscription<ur_dashboard_msgs::msg::RobotMode>::SharedPtr robot_mode_sub_;
  rclcpp::Subscription<ur_dashboard_msgs::msg::SafetyMode>::SharedPtr safety_mode_sub_;

  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr unlock_protective_stop_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr restart_safety_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr power_on_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr power_off_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr brake_release_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr stop_program_srv_;
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr play_program_srv_;
};
}  // namespace ur_robot_driver
