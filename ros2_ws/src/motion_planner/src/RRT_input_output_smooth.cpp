#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/header.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "nav_msgs/msg/path.hpp"

#include "tf2_msgs/msg/tf_message.hpp"

#include "pgm.hpp"


using std::placeholders::_1;


#include "planning_fun.h"
#include "planning_fun_terminate.h"
#include "coder_array.h"
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
static void argInit_1x2_real_T(double result[2]);
static void argInit_1x6_real_T(double result[6]);
static coder::array<unsigned char, 2U> argInit_UnboundedxUnbounded_uint8_T();
static double argInit_real_T();
static unsigned char argInit_uint8_T();
static void main_planning_fun();


// Function Definitions 

// LIMIT MAP
// Arguments    : double result[2]
// Return Type  : void
//
static void argInit_1x2_real_T(double result[2])
{
  // Loop over the array to initialize each element.
  for (int idx1 = 0; idx1 < 2; idx1++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    //result[idx1] = argInit_real_T();
    result[idx1] = 5;
  }
}

// INITIAL STATE
// Arguments    : double result[6]
// Return Type  : void
//
static void argInit_1x6_real_T(double result[6])
{
  // Loop over the array to initialize each element.
  for (int idx1 = 0; idx1 < 6; idx1++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    //result[idx1] = argInit_real_T();
    result[idx1] = 0;
  }
}

// MAP LOADING
// Arguments    : void
// Return Type  : coder::array<unsigned char, 2U>
//
static coder::array<unsigned char, 2U> argInit_UnboundedxUnbounded_uint8_T(int rows, int colums, table image_loaded)
{
  coder::array<unsigned char, 2U> result;

  // Set the size of the array.
  // Change this size to the value that the application requires.
  //result.set_size(width,height);
  result.set_size(rows,colums);

  // Loop over the array to initialize each element.
  for (int idx0 = 0; idx0 < result.size(0); idx0++) {
    for (int idx1 = 0; idx1 < result.size(1); idx1++) {
      // Set the value of the array element.
      // Change this value to the value that the application requires.
      //result[idx0 + result.size(0) * idx1] = argInit_uint8_T();
      //result[idx0 + result.size(0) * idx1] = 250;
      if(idx0 < 10 && idx1 < 10) {
      std::cout << "rows" << idx0;
      std::cout << " colums " << idx1 << std::endl;
      std::cout << "data" << image_loaded.data[idx0][idx1] << std::endl;
      std::cout << "data opposite" << image_loaded.data[idx1][idx0] << std::endl;
      }
      //RCLCPP_INFO( "read '%i'",idx0);
      //RCLCPP_INFO("read '%i'",idx1);
      //RCLCPP_INFO("read '%i'",image_loaded.data[idx0][idx1]);

      //[colonne][righe] per accesso
      result[idx0 + result.size(0) * idx1] = image_loaded.data[idx1][idx0];
    }
  }

  return result;
}

// TO USE TO LOAD DATA 
// Arguments    : void
// Return Type  : double
//
static double argInit_real_T()
{
  return 0.0;
}

// TO USE TO LOAD DATA
// Arguments    : void
// Return Type  : unsigned char
//
static unsigned char argInit_uint8_T()
{
  return 0U;
}





class MinimalSubscriber : public rclcpp::Node
{
  public:
    MinimalSubscriber()
    : Node("planner")
    {
      subscription_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
      "goal_pose", 1, std::bind(&MinimalSubscriber::topic_callback, this, _1));

      //subscription_TF = this->create_subscription<tf2_msgs::msg::TFMessage>(
      //"tf", 1, std::bind(&MinimalSubscriber::topic_callback, this, _1));
      

      publisher_ = this->create_publisher<nav_msgs::msg::Path>("path", 1);

      RCLCPP_INFO(this->get_logger(), "Node started");
    }

  private:
    void topic_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) const
    {
      RCLCPP_INFO(this->get_logger(), "Goal x: '%f'", msg->pose.position.x);
      RCLCPP_INFO(this->get_logger(), "Goal y: '%f'", msg->pose.position.y);


      //matlab integration
      coder::array<double, 2U> final_path;
      coder::array<unsigned char, 2U> image;
      double dv[6];
      double limit_tmp[2];
      double goal_tmp[2];
      double dt_tmp;
      double resolution;
      double maxIter;
      
      // Initialize function 'planning_fun' input arguments.
      
      // Initialize function input argument 'dt'.
      //dt_tmp = argInit_real_T();
      dt_tmp = 0.1;

      maxIter = 10000;

      resolution = 0.05;

      // Initialize function input argument 'limit'.
      //argInit_1x2_real_T(limit_tmp);
      limit_tmp[0] = 4;
      limit_tmp[1] = 4;

      // Initialize function input argument 'goal'.
      //argInit_1x2_real_T(goal_tmp);
      goal_tmp[0] = msg->pose.position.x;
      goal_tmp[1] = msg->pose.position.y;

      //goal_tmp[0] = goal_tmp[0] + 1.03;
      //goal_tmp[1] = goal_tmp[1] + 1.46;
      
      //goal_tmp[1] = goal_tmp[1] - 1.46;

      double temp_x = goal_tmp[0];
      double temp_y = goal_tmp[1];

      goal_tmp[0] = -temp_y;
      goal_tmp[1] = temp_x;
      
      //goal_tmp[0] = goal_tmp[0] + 1.03;
      //goal_tmp[1] = goal_tmp[1] + 1.46;

      goal_tmp[0] = goal_tmp[0] + 1.46;
      goal_tmp[1] = goal_tmp[1] + 1.03;

      //goal_tmp[0] = 2;
      //goal_tmp[1] = 2;
      if(goal_tmp[0] > limit_tmp[0])
        goal_tmp[0] = limit_tmp[0];
      if(goal_tmp[0] < 0)
        goal_tmp[0] = 0;
      if(goal_tmp[1] > limit_tmp[1])
        goal_tmp[1] = limit_tmp[1];
      if(goal_tmp[1] < 0)
        goal_tmp[1] = 0;



      RCLCPP_INFO(this->get_logger(), "Goal modified x: '%f'", goal_tmp[0]);
      RCLCPP_INFO(this->get_logger(), "Goal modified y: '%f'", goal_tmp[1]);
      


      // Initialize function input argument 'image'.
      struct table image_pgm = pgm_imread("/mnt/c/Users/giuli/Desktop/git/differential_drive/ros2_ws/coppeliasim_simple_inflated2.pgm");
      image = argInit_UnboundedxUnbounded_uint8_T(image_pgm.rows,image_pgm.cols,image_pgm);

      RCLCPP_INFO(this->get_logger(), "rows: '%i'", image_pgm.rows);
      RCLCPP_INFO(this->get_logger(), "colums: '%i'", image_pgm.cols);

      // Call the entry-point 'planning_fun'.
      //argInit_1x6_real_T(dv);
      // Initialize function input argument 'state_robot'.
      //dv[0] = 0 + 1.03;
      //dv[1] = 0 + 1.46;

      dv[0] = 0 + 1.46;
      dv[1] = 0 + 1.03;
      dv[2] = 0;
      dv[3] = 0;
      dv[4] = 0;
      dv[5] = 0;
      
      
      planning_fun(dv, dt_tmp, limit_tmp, goal_tmp, image, resolution, maxIter,final_path);


      double size_path = final_path.numel();
      RCLCPP_INFO(this->get_logger(), "Size Path: '%f'", size_path);

      nav_msgs::msg::Path gui_path;
      gui_path.poses.resize(size_path/6);
      gui_path.header.frame_id = "odom";
      //gui_path.header.stamp = 0;

      //auto pose = geometry_msgs::msg::Pose();
      //auto header = std_msgs::msg::Header();
      auto poseStamped = geometry_msgs::msg::PoseStamped();


      for(int j = 0; j < size_path/6; j++) {
        //RCLCPP_INFO(this->get_logger(), "#########");
        //double value = final_path.at(j);
        //RCLCPP_INFO(this->get_logger(), "X: '%f'", final_path[j]);
        //RCLCPP_INFO(this->get_logger(), "Y: '%f'", final_path[(size_path/6) + j]);
        //RCLCPP_INFO(this->get_logger(), "Y: '%f'", final_path[j]);

        //to plot path
        //poseStamped.pose.position.x = final_path[j] - 1.03;
        //poseStamped.pose.position.y = final_path[(size_path/6) + j] - 1.46;

        poseStamped.pose.position.x = final_path[j] - 1.46;
        poseStamped.pose.position.y = final_path[(size_path/6) + j] - 1.03;

        //to rotate!
        temp_x = poseStamped.pose.position.x;
        temp_y = poseStamped.pose.position.y;

        poseStamped.pose.position.x = temp_y;
        poseStamped.pose.position.y = -temp_x;

        RCLCPP_INFO(this->get_logger(), "X: '%f'", poseStamped.pose.position.x);
        RCLCPP_INFO(this->get_logger(), "Y: '%f'", poseStamped.pose.position.y);


        poseStamped.header.frame_id = "odom";

        gui_path.poses[j] = poseStamped;

        
      }

      publisher_->publish(gui_path);

      planning_fun_terminate();




    }
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;
    //rclcpp::Subscription<tf2_msgs::msg::PoseStamped>::SharedPtr subscription_TF;
    
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}