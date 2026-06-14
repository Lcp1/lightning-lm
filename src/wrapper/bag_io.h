//
// Created by xiang on 23-12-14.
//

#ifndef LIGHTNING_BAG_IO_H
#define LIGHTNING_BAG_IO_H

#include <functional>
#include <map>
#include <string>

#include <ros/serialization.h>
#include <rosbag/bag.h>
#include <rosbag/message_instance.h>
#include <rosbag/view.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/LaserScan.h>
#include <sensor_msgs/PointCloud2.h>

#include "common/imu.h"
#include "common/odom.h"
#include "common/point_def.h"
#include "core/lightning_math.hpp"
#include "io/dataset_type.h"
#include "wrapper/ros_utils.h"

namespace lightning {

/**
 * ROSBAG IO
 * 指定一个包名，添加一些回调函数，就可以顺序遍历这个包
 * ROS1 Noetic 版本
 */
class RosbagIO {
   public:
    explicit RosbagIO(std::string bag_file, DatasetType dataset_type = DatasetType::NCLT)
        : bag_file_(std::move(bag_file)) {
        /// handle ctrl-c
        signal(SIGINT, lightning::debug::SigHandle);
    }

    using MsgType = rosbag::MessageInstance;
    using MessageProcessFunction = std::function<bool(const MsgType &m)>;

    /// 一些方便直接使用的topics, messages
    using Scan2DHandle = std::function<bool(sensor_msgs::LaserScan::ConstPtr)>;
    using PointCloud2Handle = std::function<bool(sensor_msgs::PointCloud2::ConstPtr)>;
    using FullPointCloudHandle = std::function<bool(FullCloudPtr)>;
    using ImuHandle = std::function<bool(IMUPtr)>;
    using OdomHandle = std::function<bool(const OdomPtr &)>;

    /**
     * 遍历文件内容，调用回调函数
     * @param sleep_usec 每调用一个回调后的等待时间
     */
    void Go(int sleep_usec = 0);

    /// 通用处理函数
    RosbagIO &AddHandle(const std::string &topic_name, MessageProcessFunction func) {
        process_func_.emplace(topic_name, func);
        return *this;
    }

    /// point cloud 2 处理
    RosbagIO &AddPointCloud2Handle(const std::string &topic_name, PointCloud2Handle f) {
        return AddHandle(topic_name, [f](const MsgType &m) -> bool {
            auto msg = m.instantiate<sensor_msgs::PointCloud2>();
            if (msg != nullptr) {
                return f(msg);
            }
            return true;
        });
    }

    RosbagIO &AddImuHandle(const std::string &topic_name, ImuHandle f) {
        return AddHandle(topic_name, [f](const MsgType &m) -> bool {
            auto msg = m.instantiate<sensor_msgs::Imu>();
            if (msg != nullptr) {
                IMUPtr imu = std::make_shared<IMU>();
                imu->timestamp = ToSec(msg->header.stamp);

                /// NOTE: 如果需要乘重力，请修改此处
                imu->linear_acceleration =
                    Vec3d(msg->linear_acceleration.x, msg->linear_acceleration.y, msg->linear_acceleration.z);
                imu->angular_velocity =
                    Vec3d(msg->angular_velocity.x, msg->angular_velocity.y, msg->angular_velocity.z);

                return f(imu);
            }
            return true;
        });
    }

    /// 清除现有的处理函数
    void CleanProcessFunc() { process_func_.clear(); }

   private:
    std::map<std::string, MessageProcessFunction> process_func_;
    std::string bag_file_;
    DatasetType dataset_type_ = DatasetType::NCLT;
};
}  // namespace lightning

#endif  // SLAM_ROS_BAG_IO_H
