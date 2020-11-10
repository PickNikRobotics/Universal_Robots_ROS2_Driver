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
 * \date    2019-10-21
 *
 */
//----------------------------------------------------------------------

#pragma once

// System
#include <regex>

// ROS
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>

// UR client library
#include <ur_client_library/ur/dashboard_client.h>
#include <ur_dashboard_msgs/msg/program_state.hpp>
#include <ur_dashboard_msgs/srv/add_to_log.hpp>
#include <ur_dashboard_msgs/srv/get_loaded_program.hpp>
#include <ur_dashboard_msgs/srv/get_program_state.hpp>
#include <ur_dashboard_msgs/srv/get_robot_mode.hpp>
#include <ur_dashboard_msgs/srv/get_safety_mode.hpp>
#include <ur_dashboard_msgs/srv/is_program_running.hpp>
#include <ur_dashboard_msgs/srv/is_program_saved.hpp>
#include <ur_dashboard_msgs/srv/load.hpp>
#include <ur_dashboard_msgs/srv/popup.hpp>
#include <ur_dashboard_msgs/srv/raw_request.hpp>

namespace ur_robot_driver
{
/*!
 * \brief ROS wrapper for UR's dashboard server access. Many (not necessarily all) dashboard
 * functionalities are wrapped into ROS services here.
 */
class DashboardClientROS
{
public:
  /*!
   * \brief Constructor that shall be used by default
   *
   * \param nh Node handle pointing to the name space the dashboard-related functionalities are to
   * be found
   * \param robot_ip IP address of the robot
   */
  DashboardClientROS(const rclcpp::Node::SharedPtr& node, const std::string& robot_ip);
  DashboardClientROS() = delete;
  virtual ~DashboardClientROS() = default;

private:
  inline rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr createDashboardTriggerSrv(const std::string& topic, const std::string& command,
                                                      const std::string& expected)
  {
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service = node_->create_service<std_srvs::srv::Trigger>("topic",
      [&, command, expected](const std::shared_ptr<std_srvs::srv::Trigger::Request> req, const std::shared_ptr<std_srvs::srv::Trigger::Response> resp) {
        resp->message = this->client_.sendAndReceive(command);
        resp->success = std::regex_match(resp->message, std::regex(expected));
      });

    return service;
  }

  bool connect();

  std::shared_ptr<rclcpp::Node> node_;
  urcl::DashboardClient client_;

  // Commanding services
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr brake_release_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr clear_operational_mode_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr close_popup_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr close_safety_popup_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr pause_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr play_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr power_off_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr power_on_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr restart_safety_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr shutdown_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr unlock_protective_stop_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr running_service_;
};
}  // namespace ur_robot_driver
