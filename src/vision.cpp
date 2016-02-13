#include "ros/ros.h"
#include "geometry_msgs/Pose2D.h"
#include "walle/SoccerPoses.h"
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>

#include <cmath>
#include <iostream>
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

#define FIELD_WIDTH 3.048  // in meters
#define FIELD_HEIGHT 1.524 
#define ROBOT_RADIUS 0.10

Scalar red[]    = {Scalar(0,   128, 128), Scalar(10,  255, 255)};
Scalar yellow[] = {Scalar(20,  128, 128), Scalar(30,  255, 255)};
Scalar green[]  = {Scalar(55,  128, 128), Scalar(65,  255, 255)};
Scalar blue[]   = {Scalar(115, 128, 128), Scalar(125, 255, 255)};
Scalar purple[] = {Scalar(145, 128, 128), Scalar(155, 255, 255)};

void thresholdImage(Mat& imgHSV, Mat& imgGray, Scalar color[])
{
	inRange(imgHSV, color[0], color[1], imgGray);

	erode(imgGray, imgGray, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
	dilate(imgGray, imgGray, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
}

Point2d getCenterOfMass(Moments moment)
{
	double m10 = moment.m10;
	double m01 = moment.m01;
	double mass = moment.m00;
	double x = m10 / mass;
	double y = m01 / mass;
	return Point2d(x, y);
}

bool compareMomentAreas(Moments moment1, Moments moment2)
{
	double area1 = moment1.m00;
	double area2 = moment2.m00;
	return area1 < area2;
}

Point2d imageToWorldCoordinates(Point2d point_i, Size imageSize)
{
	Point2d centerOfField(imageSize.width / 2, imageSize.height / 2);
	Point2d center_w = (point_i - centerOfField) * (1.0 / imageSize.width * FIELD_WIDTH);
	center_w.y = -center_w.y;
	return center_w;
}

void getRobotPose(Mat& imgHsv, Scalar color[], geometry_msgs::Pose2D& robotPose)
{
	Mat imgGray;
	thresholdImage(imgHsv, imgGray, color);

	vector< vector<Point> > contours;
	vector<Moments> mm;
	vector<Vec4i> hierarchy;
	findContours(imgGray, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	if (hierarchy.size() != 2)
		return;

	for(int i = 0; i < hierarchy.size(); i++)
		mm.push_back(moments((Mat)contours[i]));

	std::sort(mm.begin(), mm.end(), compareMomentAreas);
	Moments mmLarge = mm[mm.size() - 1];
	Moments mmSmall = mm[mm.size() - 2];

	Point2d centerLarge = imageToWorldCoordinates(getCenterOfMass(mmLarge), imgHsv.size());
	Point2d centerSmall = imageToWorldCoordinates(getCenterOfMass(mmSmall), imgHsv.size());

	Point2d robotCenter = (centerLarge + centerSmall) * (1.0 / 2);
	Point2d diff = centerSmall - centerLarge;
	double angle = atan2(diff.y, diff.x);

	robotPose.x = robotCenter.x;
	robotPose.y = robotCenter.y;
	robotPose.theta = angle;
}

void getBallPose(Mat& imgHsv, Scalar color[], geometry_msgs::Pose2D& ballPose)
{
	Mat imgGray;
	thresholdImage(imgHsv, imgGray, color);

	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(imgGray, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	if (hierarchy.size() != 1)
		return;

	Moments mm = moments((Mat)contours[0]);
	Point2d ballCenter = imageToWorldCoordinates(getCenterOfMass(mm), imgHsv.size());

	ballPose.x = ballCenter.x;
	ballPose.y = ballCenter.y;
	ballPose.theta = 0;
}

ros::Publisher soccer_pub;

void processImage(Mat frame)
{
	Mat imgHsv;
	cvtColor(frame, imgHsv, COLOR_BGR2HSV);

	walle::SoccerPoses soccerPoses;

	geometry_msgs::Pose2D poseHome1;
	geometry_msgs::Pose2D poseHome2;
	geometry_msgs::Pose2D poseAway1;
	geometry_msgs::Pose2D poseAway2;
	geometry_msgs::Pose2D poseBall;

	getRobotPose(imgHsv, blue,   soccerPoses.home1);
	getRobotPose(imgHsv, green,  soccerPoses.home2);
	getRobotPose(imgHsv, red,    soccerPoses.away1);
	getRobotPose(imgHsv, purple, soccerPoses.away2);
	getBallPose(imgHsv,  yellow, soccerPoses.ball);

	soccer_pub.publish(soccerPoses);
}

void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
	try
	{
		Mat frame = cv_bridge::toCvShare(msg, "bgr8")->image;
		processImage(frame);
		imshow("Soccer Overhead Camera", frame);		
		waitKey(30);
	}
	catch (cv_bridge::Exception& e)
	{
		ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
	}
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "vision");
	ros::NodeHandle nh;

	// Subscribe to camera
	image_transport::ImageTransport it(nh);
	image_transport::Subscriber image_sub = it.subscribe("/camera1/image_raw", 1, imageCallback);

	// Publish robot locations
	soccer_pub = nh.advertise<walle::SoccerPoses>("/vision", 5);
	ros::spin();
	return 0;
}