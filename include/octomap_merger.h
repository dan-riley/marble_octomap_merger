#ifndef OCTOMAP_MERGER_H_
#define OCTOMAP_MERGER_H_

#include <Eigen/SVD>
#include <ros/ros.h>
#include <pcl/common/common.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/filter.h>
#include <pcl/features/normal_3d.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/icp_nl.h>
#include <pcl/registration/transforms.h>
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/PointCloud2.h>
#include <octomap/octomap.h>
#include <octomap/OcTreeStamped.h>
#include <octomap_msgs/Octomap.h>
#include <octomap_msgs/conversions.h>
#include <std_msgs/UInt32.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <cmath>
#include "marble_octomap_merger/OctomapArray.h"
#include "marble_octomap_merger/OctomapNeighbors.h"

using std::cout;
using std::endl;
using namespace octomap;
using namespace octomath;

// convenient typedefs
typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;

#define MAXITER 500

template <typename T>
void tree2PointCloud(T *tree, pcl::PointCloud<pcl::PointXYZ>& pclCloud) {
  for (typename T::leaf_iterator it = tree->begin_leafs(),
       end = tree->end_leafs(); it != end; ++it)
  {
    if (tree->isNodeOccupied(*it)) {
      pclCloud.push_back(
          pcl::PointXYZ(it.getX(),
            it.getY(),
            it.getZ()
            )
          );
    }
  }
}

bool pointInBBox(pcl::PointXYZ& point,
                 pcl::PointXYZ& bboxMin,
                 pcl::PointXYZ& bboxMax);

Eigen::Matrix4f getICPTransformation(
    pcl::PointCloud<pcl::PointXYZ>& cloud1,
    pcl::PointCloud<pcl::PointXYZ>& cloud2,
    Eigen::Matrix4f& tfEst,
    double mapRes);

void transformTree(OcTree *tree, Eigen::Matrix4f& transform);

void align_maps(OcTree *tree1, OcTree *tree2, point3d translation,
                double roll, double pitch, double yaw, double res);

double build_diff_tree(OcTree *tree1, OcTree *tree2, OcTree *tree_diff);
void merge_maps(OcTreeStamped *tree1, OcTree *tree2, bool replace, bool overwrite);

class OctomapMerger {
  public:
    // Constructor
    OctomapMerger(ros::NodeHandle* nodehandle);
    // Destructor
    ~OctomapMerger();
    // Callbacks
    void callback_myMap(const octomap_msgs::Octomap::ConstPtr& msg);
    void callback_neighborMaps(const marble_octomap_merger::OctomapNeighborsConstPtr &msg);
    // Public Methods
    void merge();
    void combine_diffs();
    // Variables
    bool myMapNew;
    bool otherMapsNew;
    std::string id;
    std::string type;
    int merger;
    bool full_merge;
    bool free_prioritize;
    int octo_type;
    double resolution;
    int map_thresh;
    std::string map_topic;
    std::string neighbors_topic;
    std::string merged_topic;
    std::string map_diffs_topic;
    std::string num_diffs_topic;
    std::string pcl_topic;

  /* Private Variables and Methods */
  private:
    ros::NodeHandle nh_;

    octomap_msgs::Octomap myMap;
    marble_octomap_merger::OctomapArray mapdiffs;
    marble_octomap_merger::OctomapNeighbors neighbors;
    octomap::OcTreeStamped *tree_merged;
    octomap::OcTree *tree_sys;
    octomap::OcTree *tree_old;
    octomap::OcTree *tree_temp;
    octomap::OcTree *tree_diff;
    int num_diffs;
    std::map<std::string, std::vector<int>> seqs;

    ros::Subscriber sub_mymap;
    ros::Subscriber sub_neighbors;

    ros::Publisher pub_merged;
    ros::Publisher pub_size;
    ros::Publisher pub_mapdiffs;
    ros::Publisher pub_pcl;

    void initializeSubscribers();
    void initializePublishers();
};

#endif
