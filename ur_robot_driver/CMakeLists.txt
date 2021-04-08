cmake_minimum_required(VERSION 3.5)
project(ur_robot_driver)

add_compile_options(-std=c++11)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  message("${PROJECT_NAME}: You did not request a specific build type: selecting 'RelWithDebInfo'.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)
find_package(ur_client_library REQUIRED)
find_package(ur_dashboard_msgs REQUIRED)

include_directories(
  include
)

set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  std_srvs
  ur_client_library
  ur_dashboard_msgs
)

add_library(ur_robot_driver_plugin
  SHARED
  src/dashboard_client_ros.cpp
  src/hardware_interface.cpp
  src/robot_state_helper.cpp
)
target_link_libraries(
  ur_robot_driver_plugin
  ur_client_library::urcl
)
target_include_directories(
  ur_robot_driver_plugin
  PRIVATE
  include
)
ament_target_dependencies(
  ur_robot_driver_plugin
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)
pluginlib_export_plugin_description_file(hardware_interface hardware_interface_plugin.xml)

# add_executable(ur_robot_driver_node
#   src/dashboard_client_ros.cpp
#   src/hardware_interface.cpp
#   src/hardware_interface_node.cpp
# )
# target_link_libraries(ur_robot_driver_node ${catkin_LIBRARIES} ur_client_library::urcl)
# ament_target_dependencies(ur_robot_driver_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${THIS_PACKAGE_INCLUDE_DEPENDS})

add_executable(dashboard_client
  src/dashboard_client_ros.cpp
  src/dashboard_client_node.cpp
)
target_link_libraries(dashboard_client ${catkin_LIBRARIES} ur_client_library::urcl)
ament_target_dependencies(dashboard_client ${${PROJECT_NAME}_EXPORTED_TARGETS} ${THIS_PACKAGE_INCLUDE_DEPENDS})

install(
        TARGETS dashboard_client
        DESTINATION lib/${PROJECT_NAME}
)

# add_executable(robot_state_helper
#   src/robot_state_helper.cpp
#   src/robot_state_helper_node.cpp
# )
# target_link_libraries(robot_state_helper ${catkin_LIBRARIES} ur_client_library::urcl)
# ament_target_dependencies(robot_state_helper ${${PROJECT_NAME}_EXPORTED_TARGETS} ${THIS_PACKAGE_INCLUDE_DEPENDS})

# install(TARGETS ur_robot_driver_plugin ur_robot_driver_node robot_state_helper dashboard_client
#   ARCHIVE DESTINATION lib}
#   LIBRARY DESTINATION lib}
#   RUNTIME DESTINATION bin}
# )

# INSTALL
install(
        TARGETS ur_robot_driver_plugin
        DESTINATION lib
)
install(
        DIRECTORY include/
        DESTINATION include
)


## EXPORTS
ament_export_include_directories(
        include
)
ament_export_libraries(
        ur_robot_driver_plugin
)

install(PROGRAMS scripts/tool_communication
  DESTINATION bin
)

install(DIRECTORY config launch resources
  DESTINATION share/${PROJECT_NAME}
)


install(DIRECTORY include/
  DESTINATION include
)

ament_export_dependencies(
        hardware_interface
        pluginlib
        rclcpp
        ${THIS_PACKAGE_INCLUDE_DEPENDS}
)

ament_package()
