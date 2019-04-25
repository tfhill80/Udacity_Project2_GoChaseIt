#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Driving the Robot");    

    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int white_pixel_index = -1;
    int pixel_location = -1;
    int divresult; //needed for pixel location calculation

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    // Loop through each pixel in the image and check if its equal to the first one
    for (int i=0; i < img.height; i++) {
      for (int j=0; j < img.step; j++) {
        //check all three channels at once
        if((img.data[i*img.step + j] == white_pixel)&&(img.data[i*img.step + j + 1] == white_pixel)&&(img.data[i*img.step + j + 2] == white_pixel)) {
            pixel_location = j; //j represents the column data. This is what we need to partition between left/right/center drive
            break;
        }
      }
    }

    /*
    for (int i = 0; i < img.height * img.step; i++) {
       if(img.data[i] == white_pixel){
	   //need to check that all 3 channels are white. Assume that first hit is channel 1.
           if((img.data[i+1] == white_pixel) && (img.data[i+2]==white_pixel)) { //check other channels
           white_pixel_index = i;
           break;
           }
       }
    }
    if (white_pixel_index == -1){ //No pixel found
       pixel_location = -1;
    } 
    else{
        // white pixel % height tells you which matrix column; since pixels are in triplets the next step is to div my step size.
        divresult = white_pixel_index / img.step; //this will tell us the pixel number
        pixel_location = divresult % img.height; //this will give the correct pixel column
    } */

    ROS_INFO("White Pixel Index at %i", pixel_location);
    if (pixel_location == -1){ //Stop Condition, no ball is seen
       drive_robot(0.0,0.0); //stop
    }
    else if (pixel_location <= (int)(img.step / 3)){ //left side condition, drive left
       ROS_INFO("Left Side, less than column at %i", (int)(img.step / 3));
       drive_robot(0.0,1.0); //drive left - old command was 0, 0.5
    } 
    else if (pixel_location >= (int)(2* img.step / 3)){ //right side condition, drive right
       ROS_INFO("Right Side, greater than column at %i", (int)(2* img.step / 3));
       drive_robot(0.0,-1.0); //drive right - old command was 0, -0.5
    } 
    else { //drive forward
       drive_robot(1.0,0.0); //drive forward - old command was 0.5, 0
    } 


}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
