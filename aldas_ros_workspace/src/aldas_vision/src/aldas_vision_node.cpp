#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <chrono>
//for opencv and cv_bridge
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

class AldasVisionNode : public rclcpp::Node {

private:
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr camera_subscriber_;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_subscriber_;
    //pub for controller script x,y error.
    rclcpp::Publisher<geometry_msgs::msg::Point>::SharedPtr target_error_publisher_;

    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;
    bool camera_info_received_ = false;

    //aruco params
    cv::Ptr<cv::aruco::Dictionary> aruco_dict_;
    cv::Ptr<cv::aruco::DetectorParameters> aruco_params_;
    float marker_length_ = 0.2f; // 20cm marker, adjust accordingly

    void camera_info_callback(const sensor_msgs::msg::CameraInfo::SharedPtr msg) {
        if (!camera_info_received_) {
            camera_matrix_ = cv::Mat(3, 3, CV_64F, (void*)msg->k.data()).clone();
            dist_coeffs_ = cv::Mat(1, 5, CV_64F, (void*)msg->d.data()).clone();
            camera_info_received_ = true;
            RCLCPP_INFO(this->get_logger(), "Camera data recieved.");
        }
    }

    void camera_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        if (!camera_info_received_) return; // we need the camera info.

        cv_bridge::CvImagePtr cv_ptr;
        try {
            // Convert ROS image message to OpenCV Matrix 
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
            return;
        }

        //aruco detection logic

        std::vector<int> marker_ids;
        std::vector<std::vector<cv::Point2f>> marker_corners, rejected_candidates;

        // Detect the marker in image
        cv::aruco::detectMarkers(cv_ptr->image, aruco_dict_, marker_corners, marker_ids, aruco_params_, rejected_candidates);

        geometry_msgs::msg::Point target_error;

        if (!marker_ids.empty()) {
            std::vector<cv::Vec3d> rvecs, tvecs;
            
            // estimate Pose (Returns X, Y, Z translation relative to camera)
            cv::aruco::estimatePoseSingleMarkers(marker_corners, marker_length_, camera_matrix_, dist_coeffs_, rvecs, tvecs);

            // Extract data for the controller
            target_error.x = tvecs[0][0]; // x axis error
            target_error.y = tvecs[0][1]; // y axis error
            target_error.z = 1.0;         // 1.0 flag means target is locked

            // Draw bounding box and 3D axis on the image for visual debugging
            cv::aruco::drawDetectedMarkers(cv_ptr->image, marker_corners, marker_ids);
            cv::drawFrameAxes(cv_ptr->image, camera_matrix_, dist_coeffs_, rvecs[0], tvecs[0], 0.1);
        } else {
            // Target lost
            target_error.x = 0.0;
            target_error.y = 0.0;
            target_error.z = 0.0;
        }

        // publish the error coords
        target_error_publisher_->publish(target_error);

        // for cam feed
        cv::imshow("ArUco Tracking", cv_ptr->image);
        cv::waitKey(1);
    }


public:
    AldasVisionNode() : Node("aldas_vision_node") {
        // Initialize subscribers and publishers

        target_error_publisher_ = this->create_publisher<geometry_msgs::msg::Point>(
            "/aldas/target_error", 10);

        //subs for gazebo sensors
        camera_subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw", 10,
            std::bind(&AldasVisionNode::camera_callback, this, std::placeholders::_1));

        camera_info_subscriber_ = this->create_subscription<sensor_msgs::msg::CameraInfo>(
            "/camera/camera_info", 10,
            std::bind(&AldasVisionNode::camera_info_callback, this, std::placeholders::_1));

            //Aruco params
            aruco_dict_ = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
            aruco_params_ = cv::aruco::DetectorParameters::create();
            RCLCPP_INFO(this->get_logger(), "Aldas Vision Node initialized.");
    }
};
int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<AldasVisionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}