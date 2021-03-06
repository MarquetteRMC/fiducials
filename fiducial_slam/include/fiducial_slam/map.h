/*
 * Copyright (c) 2017-8, Ubiquity Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 *
 */
#ifndef MAP_H
#define MAP_H

#include <ros/ros.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <visualization_msgs/Marker.h>

#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include <fiducial_msgs/FiducialMapEntry.h>
#include <fiducial_msgs/FiducialMapEntryArray.h>

#include <list>
#include <string>

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <tf2/convert.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <std_srvs/Empty.h>

#include <fiducial_slam/transform_with_variance.h>

using namespace std;
using namespace cv;

// An observation of a single fiducial in a single image
class Observation {
  public:
    int fid;
    tf2::Stamped<TransformWithVariance> T_fidCam;
    tf2::Stamped<TransformWithVariance> T_camFid;

    // how well this fitted the consensus of cameraPose
    tf2::Vector3 position;

    Observation() {};

    Observation(int fid, const tf2::Stamped<TransformWithVariance>& camFid);
};

// A single fiducial that is in the map
class Fiducial {
  public:
    int id;
    int numObs;
    bool visible;
    map<int,int> links;

    tf2::Stamped<TransformWithVariance> pose;
    ros::Time lastPublished;

    void update(const tf2::Stamped<TransformWithVariance>& newPose);

    Fiducial() {}

    Fiducial(int id, const tf2::Stamped<TransformWithVariance>& pose);
};

// Class containing map data
class Map {
  public:
    tf2_ros::TransformBroadcaster broadcaster;
    tf2_ros::Buffer tfBuffer;
    unique_ptr<tf2_ros::TransformListener> listener;

    ros::Publisher markerPub;
    ros::Publisher mapPub;
    ros::Publisher robotPosePub;
    ros::Publisher cameraPosePub;

    ros::ServiceServer clearSrv;
    bool clearCallback(std_srvs::Empty::Request &req,
                       std_srvs::Empty::Response &res);
    string mapFilename;
    string mapFrame;
    string odomFrame;
    string cameraFrame;
    string baseFrame;
    double future_date_transforms;
    bool publish_6dof_pose;
    double multiErrorThreshold;

    bool isInitializingMap;
    bool readOnly;
    int frameNum;
    int initialFrameNum;
    int originFid;

    bool overridePublishedCovariance;
    std::vector<double> covarianceDiagonal;

    bool havePose;
    float tfPublishInterval;
    bool publishPoseTf;
    ros::Time tfPublishTime;
    geometry_msgs::TransformStamped poseTf;

    map<int, Fiducial> fiducials;

    Map(ros::NodeHandle &nh);
    void update();
    void update(vector<Observation> &obs, const ros::Time &time);
    void autoInit(const vector<Observation> &obs, const ros::Time &time);
    int  updatePose(vector<Observation> &obs, const ros::Time &time,
                    tf2::Stamped<TransformWithVariance>& cameraPose);
    void updateMap(const vector<Observation> &obs, const ros::Time &time,
                   const tf2::Stamped<TransformWithVariance>& cameraPose);

    bool loadMap();
    bool loadMap(std::string filename);
    bool saveMap();
    bool saveMap(std::string filename);

    void publishTf();
    void publishMap();
    void publishMarker(Fiducial &fid);
    void publishMarkers();
    void drawLine(const tf2::Vector3 &p0, const tf2::Vector3 &p1);

    bool lookupTransform(const std::string &from, const std::string &to,
                         const ros::Time &time, tf2::Transform &T) const;
};

#endif
