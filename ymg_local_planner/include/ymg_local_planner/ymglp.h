#ifndef YMGLP_H_
#define YMGLP_H_

#include <vector>
#include <Eigen/Core>


//for creating a local cost grid
#include <base_local_planner/map_grid_visualizer.h>
#include <pcl_ros/publisher.h>

//for obstacle data access
#include <costmap_2d/costmap_2d.h>

#include <base_local_planner/trajectory.h>
#include <base_local_planner/local_planner_limits.h>
#include <base_local_planner/local_planner_util.h>

#include <ymg_local_planner/YmgLPConfig.h>
#include <ymg_local_planner/simple_trajectory_generator_kai.h>
#include <ymg_local_planner/map_grid_cost_function_kai.h>
#include <ymg_local_planner/obstacle_cost_function_kai.h>
#include <ymg_local_planner/simple_scored_sampling_planner_kai.h>
// #include <ymg_local_planner/robot_status_manager.h>
#include <ymg_local_planner/robot_status_manager_mk2.h>
#include <ymg_local_planner/ymg_sampling_planner.h>
#include <ymg_local_planner/simple_backup_planner.h>
#include <ymg_local_planner/ymg_s_planner.h>
#include <ymg_local_planner/direction_adjust_planner.h>
#include <ymg_local_planner/util_functions.h>

#include <nav_msgs/Path.h>

namespace ymglp {

/**
	* @class ymglp
	* @brief A class implementing a local planner using the Dynamic Window Approach
	*/
class YmgLP {
		public:
				/**
					* @brief  Constructor for the planner
					* @param name The name of the planner 
					* @param costmap_ros A pointer to the costmap instance the planner should use
					* @param global_frame the frame id of the tf frame to use
					*/
				YmgLP(std::string name, base_local_planner::LocalPlannerUtil *planner_util);

				/**
					* @brief  Destructor for the planner
					*/
				~YmgLP() {if(traj_cloud_) delete traj_cloud_;}

				/**
					* @brief Reconfigures the trajectory planner
					*/
				void reconfigure(YmgLPConfig &cfg);

				/**
					* @brief  Check if a trajectory is legal for a position/velocity pair
					* @param pos The robot's position
					* @param vel The robot's velocity
					* @param vel_samples The desired velocity
					* @return True if the trajectory is valid, false otherwise
					*/
				bool checkTrajectory(
								const Eigen::Vector3f pos,
								const Eigen::Vector3f vel,
								const Eigen::Vector3f vel_samples);

				/**
					* @brief Given the current position and velocity of the robot, find the best trajectory to exectue
					* @param global_pose The current position of the robot 
					* @param global_vel The current velocity of the robot 
					* @param drive_velocities The velocities to send to the robot base
					* @return The highest scoring trajectory. A cost >= 0 means the trajectory is legal to execute.
					*/
				base_local_planner::Trajectory findBestPath(
								tf::Stamped<tf::Pose> global_pose,
								tf::Stamped<tf::Pose> global_vel,
								tf::Stamped<tf::Pose>& drive_velocities,
								std::vector<geometry_msgs::Point> footprint_spec);

				/**
					* @brief  Take in a new global plan for the local planner to follow, and adjust local costmaps
					* @param  new_plan The new global plan
					*/
				void updatePlanAndLocalCosts(tf::Stamped<tf::Pose> global_pose,
								const std::vector<geometry_msgs::PoseStamped>& new_plan);

				/**
					* @brief Get the period at which the local planner is expected to run
					* @return The simulation period
					*/
				double getSimPeriod() { return sim_period_; }

				/**
					* @brief Compute the components and total cost for a map grid cell
					* @param cx The x coordinate of the cell in the map grid
					* @param cy The y coordinate of the cell in the map grid
					* @param path_cost Will be set to the path distance component of the cost function
					* @param goal_cost Will be set to the goal distance component of the cost function
					* @param occ_cost Will be set to the costmap value of the cell
					* @param total_cost Will be set to the value of the overall cost function, taking into account the scaling parameters
					* @return True if the cell is traversible and therefore a legal location for the robot to move to
					*/
				bool getCellCosts(int cx, int cy, float &path_cost, float &goal_cost, float &occ_cost, float &total_cost);

				/**
					* sets new plan and resets state
					*/
				bool setPlan(const std::vector<geometry_msgs::PoseStamped>& orig_global_plan);

		private:

				base_local_planner::LocalPlannerUtil *planner_util_;

				double pdist_scale_, gdist_scale_, occdist_scale_;
				Eigen::Vector3f vsamples_;

				double sim_period_;///< @brief The number of seconds to use to compute max/min vels for dwa
				base_local_planner::Trajectory result_traj_;

				int nearest_index_;
				std::vector<geometry_msgs::PoseStamped> global_plan_;

				boost::mutex configuration_mutex_;
				pcl::PointCloud<base_local_planner::MapGridCostPoint>* traj_cloud_;
				pcl_ros::Publisher<base_local_planner::MapGridCostPoint> traj_cloud_pub_;
				bool publish_cost_grid_pc_; ///< @brief Whether or not to build and publish a PointCloud
				bool publish_traj_pc_;

				void publishTrajPC(std::vector<base_local_planner::Trajectory>& all_explored);

				// see constructor body for explanations
				base_local_planner::SimpleTrajectoryGeneratorKai generator_;
				base_local_planner::ObstacleCostFunctionKai obstacle_costs_;
				base_local_planner::MapGridCostFunctionKai path_costs_;
				base_local_planner::MapGridCostFunctionKai goal_costs_;
				UtilFcn utilfcn_;

				ros::Time backup_start_time_;
				bool backup_latch_;
				double stuck_timeout_, backup_time_;
				// RobotStatusManager robot_status_manager_;
				RobotStatusManagerMk2 status_manager_;

				bool use_dwa_, reverse_mode_;
				base_local_planner::SimpleScoredSamplingPlannerKai scored_sampling_planner_;
				YmgSamplingPlanner ymg_sampling_planner_;
				SimpleBackupPlanner simple_backup_planner_;

				double local_goal_distance_;
				ros::Publisher local_goal_pub_;

};   // class ymglp

}   // namespace ymglp

#endif
