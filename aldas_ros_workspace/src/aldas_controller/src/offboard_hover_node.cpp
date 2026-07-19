// Author: Shashi Bhushan Sharma

#include<rclcpp/rclcpp.hpp>
#include<stdint.h>
#include<chrono>

// Essential px4_msgs includes
#include<px4_msgs/msg/distance_sensor.hpp>
#include<px4_msgs/msg/offboard_control_mode.hpp>
#include<px4_msgs/msg/trajectory_setpoint.hpp>
#include<px4_msgs/msg/vehicle_command.hpp>

/**
 * @brief Author: Shashi Bhushan Sharma
 * @brief This node is responsible for sending offboard control commands to the PX4 autopilot.
 * @brief It publishes the necessary messages to control the drone's position, velocity, and orientation in offboard mode. 
 * @brief The node also handles the arming and disarming of the drone, as well as setting.
 */

 using namespace std::chrono_literals;

 class OffboardHoverNode: public rclcpp:: Node
 {
    public:
        OffboardHoverNode() : Node("Offboard_hover_node"){
            // The QoS setting --> The PX4 requires SensorDataQoS
            rclcpp::QoS qos_profile = rclcpp:: SensorDataQoS();

            // Lidar subscriber --> Listener that would listen the distance sensor data from the lidar
            lidar_sub_ = this->create_subscription<px4_msgs::msg::DistanceSensor>(
                "/fmu/out/distance_sensor",
                qos_profile,
                std::bind(&OffboardHoverNode::lidar_callback, this, std::placeholders::_1)
            );

            // OffboardControlMode publisher --> Publisher that would publish the offboard control node message to the PX4 autopilot
            Offboard_control_mode_pub = this->create_publisher<px4_msgs::msg::OffboardControlMode>(
                "/fmu/in/offboard_control_mode",10
            );

            // TrajectorySetPoint publisher --> pUblisher that would publish the trajectory setppoint message ot the PX4 autopilot
            Trajectory_setpoint_pub = this->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
                "/fmu/in/trajectory_setpoint", 10
            );


            // VehicleCommand publisher --> Publisher that would publish the vehicle command message to the PX4 autopilot
            Vehicle_command_pub = this->create_publisher<px4_msgs::msg::VehicleCommand>(
                "/fmu/in/vehicle_command", 10
            );

            // Timer block with 20ms heartbeat --> This timer would call the timer_callback function every 20ms to send the offboard control commands to the PX4 autopilot
            timer_ = this->create_wall_timer(
                50ms, std::bind(&OffboardHoverNode::timer_callback, this)
            );
        }


    private:
        
        // Network Object Declarations
        rclcpp::Subscription<px4_msgs::msg::DistanceSensor>::SharedPtr lidar_sub_;
        rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>:: SharedPtr Offboard_control_mode_pub;
        rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>:: SharedPtr Trajectory_setpoint_pub;
        rclcpp::Publisher<px4_msgs::msg::VehicleCommand>:: SharedPtr Vehicle_command_pub;
        rclcpp::TimerBase::SharedPtr timer_;

        // Initialising the State Variables --> These variables would be used to store the current altitude and the flight data
        float current_Altitude = 0.0;
        int flight_state_ = 0;
        int timer_count_ = 0;
        rclcpp::Time hover_start_time_; // this timer variable will be used to check the hover time of the drone.

        void lidar_callback(const px4_msgs::msg::DistanceSensor::SharedPtr msg){
            // This callback function would be called whenever a new distance sensor message is received from the lidar
            // This would update the state variable current_Altitude with the latest altitude data from the lidar
            current_Altitude = msg->current_distance;
        }

        // Helper function to send the vehicle command message to the PX4 autopilot
        void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2=0.0){
            px4_msgs::msg::VehicleCommand cmd_msg;
            cmd_msg.param1 = param1;
            cmd_msg.param2 = param2;
            cmd_msg.command = command;
            cmd_msg.target_system = 1; // Drone System ID
            cmd_msg.target_component = 1; // Flight controller ID
            cmd_msg.source_system = 1; // This computer ID
            cmd_msg.source_component = 1; // This node ID
            cmd_msg.from_external = true; 
            Vehicle_command_pub->publish(cmd_msg);
        }

        // TImer callback function --> This function would be called every 20ms to send the offboard control commands to the PX4 autopilot
        void timer_callback(){

            // 1. Always publishing the OffboardControlMode message at a 20ms heartbeat
            px4_msgs::msg::OffboardControlMode node_msg;
            node_msg.timestamp = this->get_clock()->now().nanoseconds()/1000;
            node_msg.position = true;
            node_msg.velocity = false;
            node_msg.acceleration = false;
            node_msg.attitude = false;
            node_msg.body_rate = false;
            Offboard_control_mode_pub->publish(node_msg);

            // 2. Publish the baseline TrajectorySetPoint message at a 20ms heartbeat 
            px4_msgs::msg::TrajectorySetpoint traj_msg;
            traj_msg.timestamp = this->get_clock()->now().nanoseconds()/1000.0;
            traj_msg.position[0] = 0.0; // X direction
            traj_msg.position[1] = 0.0; // Y direction
            traj_msg.position[2] = -2.0; // -z upward direction
            traj_msg.yaw = 0.0;

            timer_count_++; // increasing the tiemr count every 50ms

            // 3. Armin the drone after 2 seconds of the node start
            switch(flight_state_){
                case 0:
                    // Case 0: Waking and arming the bot after 2 seconds of continuous offboardControlMode heartbeat
                    if(timer_count_ == 30){
                        RCLCPP_INFO(this->get_logger(), "Switching to offboard Mode ... ");
                        // publishing the vehicle command message to switch to the offboard mode
                        publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);

                    }
                    else if(timer_count_ == 40){
                        RCLCPP_INFO(this->get_logger(), "Arming the drone ...");
                        // publishing the vehicle command message to arm the drone
                        publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1);

                        // changing the flight state to climb mode
                        flight_state_ = 1;
                    }

                    break;

                case 1: 
                    // Case 1: Climbing the dron 2m and monitoring lidar to equalise at that point 
                    if(timer_count_ % 10 == 0){
                        RCLCPP_INFO(this->get_logger(), "Current Altitude: %.3f cm", current_Altitude);
                    }
                    // we will check the lidar value and give a tiny margin of error that too only 5 cm
                    if(current_Altitude >= 1.950){
                        RCLCPP_INFO(this->get_logger(), "Reached the desired height of approx 2m, hovering now ...");
                        hover_start_time_ = this->get_clock()->now();  // starting the hover timer 
                        flight_state_= 2; // changing the flight state to hover mode
                    }
                    break;

                case 2:
                    // Case 2: it is the hovering mode in which the drone hover at the 2mm height reached in the previous state
                    if((this->get_clock()->now() - hover_start_time_).seconds() >= 5.0){
                        RCLCPP_INFO(this->get_logger()," HOvering is completed and now descending the drone to the land ...");
                        flight_state_ = 3; // changing the state to the descending state
                    }
                    break;

                case 3:
                    // Case 3: Descending the drone to land and disarming it after landing
                    traj_msg.position[2] = 0.0;  // This is the landing position +Z

                    if(current_Altitude <= 0.17){
                        RCLCPP_INFO(this->get_logger(), "Landing is completed and now Disarming the droibne ...");
                        publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0); // Disarming the drone
                        flight_state_ = 4;
                    }
                    break;

                case 4: // finished the offboard control and disarmed the drone
                    RCLCPP_INFO(this->get_logger(), "Offboard Control is completed and drone is disarmed ...");
                    break;
            }

            Trajectory_setpoint_pub->publish(traj_msg);


        }


 };

//  Main function --> This function would initialise the ROS2 node and spin it to keep it alive
int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardHoverNode>());
    rclcpp::shutdown();
    return 0;
}


