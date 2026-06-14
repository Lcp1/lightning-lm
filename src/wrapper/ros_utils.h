//
// Created by xiang on 25-3-24.
//

#ifndef LIGHTNING_ROS_UTILS_H
#define LIGHTNING_ROS_UTILS_H

#include <pcl_conversions/pcl_conversions.h>
#include <ros/time.h>

#include "common/point_def.h"

namespace lightning {

inline double ToSec(const ros::Time &time) { return double(time.sec) + 1e-9 * time.nsec; }
inline uint64_t ToNanoSec(const ros::Time &time) { return time.sec * 1e9 + time.nsec; }

}  // namespace lightning

#endif  // LIGHTNING_ROS_UTILS_H
