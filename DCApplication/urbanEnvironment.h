#ifndef URBAN_ENVIRONMENT_H
#define URBAN_ENVIRONMENT_H

#include "ns3/vector.h"
#include "ns3/application.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"

using namespace ns3;


//节点的移动性配置文件，由mytraceExporter.py生成，在配置节点移动性时使用
#define NODESMOBILITYFILE "/home/mak/Code/ns3-wave/cross/mobility.tcl"

//无人驾驶车辆的类型
#define AutonomousVehicle 1
#define MannedVehicle 2


/**
 * 存储由traceExporter.py生成的tcl信息
 *  (时间)*(vehicle or person)*(节点的id)*(节点的位置x)*(节点的位置y)*(运动方向angle)*(速度speed)*车道 
*/
typedef struct 
{
    //时间
    double time;
    //节点类型
    std::string nodeType;
    //节点id
    std::string nodeID;
    //节点位置x
    double locationX;
    //节点位置y
    double locationY;
    //节点运动方向
    int angle;
	//节点速度
    double speed;
	//节点的车道信息
	std::string lane;

} parsingNodeInfo;


/**
 * 邻居无人驾驶车辆节点的信息
 * PS：无人驾驶与有人驾驶的区别是，无人驾驶的信息更加完备，且无人驾驶需要计算网络性能因素
*/
typedef struct 
{
    //节点的ID，这个ID不是SUMO中节点的ID（SUMO中的ID是string类型的，不方便在packet中传输），
    //这个ID是NS3中节点的ID，NS3在Create节点后为每一个节点分配表一个ID，其数据类型为uint32_t
    uint32_t neighbor_ID;    

    //邻居节点是无人驾驶车辆(1)还是有人驾驶车辆(2)
    //int m_neighborIsAAutonomousOrMannedVehicle;

    //邻居节点的mac地址
    Address neighbor_mac;
    //第一次与邻居节点建立通信的时间
    Time neighbor_first_bacon;
    //邻居节点最后更新的时间
    Time neighbor_last_beacon;
    //邻居节点位置
    Vector neighbor_currentPosition;
    //邻居节点的速度
    Vector neighbor_currentVelocity;
    //邻居节点的运动方向
    int neighbor_angle;

    //邻居节点发送数据的延迟
    Time neighbor_messageDelay;
    //邻居节点发送数据延迟的变化度
    Time neighbor_Delay_jitter;
    //通信中断概率，正常通信时中断概率为 0，被移除时中断概率为 1
    int CommunMeToNeigBreakProb;
    //数据正确交付率，正常通信时数据正确交付率为 1，被移除时中断概率为 0
    double deliveryRate; //--->需要收到确认信息3后计算
    //链路容量，固定值
    int channelCapacity;
    //连通时长被移除时计算最长时间
    Time linkLifeTime;
    //通信往返时间，使用不同类型数据包计算通信往返时间
    Time RTT;  //--->需要收到确认信息3后计算

    //邻居节点的引领节点度
    double leaderNodeDegree;
    //邻居节点管理的节点个数
    uint32_t managedNodeCounts; 
    //AutonomousVehicleApplication中无人驾驶车辆在车群中的角色
    uint32_t AVPRole;
    //无人驾驶车辆距离引领节点或CH的跳数
    uint32_t hop2Leader;

    //PMC算法
    // bool tryConnection; //表示节点是否已经请求被连接
    double AvgRelM; //速度差异
    uint32_t following_degree; //跟随度

    //DC算法
    uint32_t neighbor_counts; //无人驾驶车辆节点邻居节点个数

    double m_score;

}NeighborAutonomousVehicleInfo;

/*
    邻居有人驾驶车辆节点的信息
*/
typedef struct 
{
    //节点的ID，这个ID不是SUMO中节点的ID（SUMO中的ID是string类型的，不方便在packet中传输），
    //这个ID是NS3中节点的ID，NS3在Create节点后为每一个节点分配表一个ID，其数据类型为uint32_t
    uint32_t neighbor_ID;
    
    //邻居节点的mac地址
    Address neighbor_mac;
    //第一次与邻居节点建立通信的时间
    Time neighbor_first_bacon;
    //邻居节点最后更新的时间
    Time neighbor_last_beacon;
    //邻居节点位置
    Vector neighbor_currentPosition;
    //邻居节点的速度
    Vector neighbor_currentVelocity;
    //邻居节点的运动方向
    int neighbor_angle;

}NeighborMannedVehicleInfo;

/**
 * 障碍物信息表
*/
typedef struct 
{
    //障碍物的属性
    //地理位置
	Vector position;

	//长度
	double length;
	//宽度
	double width;
	
} ObstacleInfo;

/**
 * 红绿灯信息表
*/
typedef struct 
{
    Vector position;

    //红绿灯的属性
	//当前状态
	// int state;
	// //当前状态剩余时间
	// int theRestTime;
    // //红灯的时间长度
    // int redLightingLength;
    // //绿灯的时间长度
    // int greenLightingLength;

} LightingInfo;

/**
 * 行人信息表
*/
typedef struct 
{
    //行人的属性
	//速度
	double speed;
	//行进方向
	double direction;
	//地理位置x
	double location_x;
	//地理位置y
	double location_y;

} PedestrianInfo;


//多目标优化算法中的个体
typedef struct 
{
    //个体对应的无人驾驶车群模型的解
    std::vector <uint32_t> individual_x;
    //种群中被individual_x支配的解的序号
    std::vector <uint32_t> S_p;
    //种群中支配解individual_x的个数
    uint32_t n_p;
    //解individual_x的rank
    uint32_t rank_individual_x;
    //解individual_x的连通性
    double connectivity_individual_x;
    //解individual_x的稳定性
    double stability_individual_x;
    //解individual_x的实时性
    double realTime_individual_x;

} IndividualInfo;

//无人驾驶车辆节点之间的通信信息及其网络性能影响因素信息
typedef struct 
{
    //仿真时间
    double s_time;
    //节点1的ID
    int n_1_ID;
    //节点2的ID
    int n_2_ID;
    //节点1的位置x
    double location_1_X;
    //节点1的位置y
    double location_1_Y;
    //节点1的速度
    double speed_1;
    //节点2的位置x
    double location_2_X;
    //节点2的位置y
    double location_2_Y;
    //节点2的速度
    double speed_2;
    //节点1和节点2之间的七大网络性能影响因素
    //发送数据的延迟 GetMilliSeconds
    double neighbor_messageDelay;
    //延迟的变化度 GetMilliSeconds
    double neighbor_Delay_jitter;
    //通信中断概率
    int CommunMeToNeigBreakProb;
    //数据正确交付率
    double deliveryRate;
    //链路容量
    int channelCapacity;
    //连通时长 GetSeconds
    double linkLifeTime;
    //通信往返时间 GetSeconds
    double RTT;

} CommunicationInfo;

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

//节点的角色
#define CORE 1 //簇头状态
#define ORDINARY 2 //成员状态
#define FREE 3 //孤立节点状态


//收发数据的类型
#define BEACON 1  //广播消息，
// #define JOINREQ 2 //点对点消息，车辆请求引领节点或普通节点加入车群
#define JOINRESP 3 //点对点消息，车辆收到引领节点发送的加入车群的回复
#define CORE2ORDINARY 4 //点对点消息，证明车群的CH存在
#define ORDINARY2CORE 5 //点对点消息，证明车群中CM存在
#define ORDINARYUP2CORE 6 //次级引领节点晋升为引领节点

#define MIN_DENSITY 4 //基于DBSCAN的聚类中节点的最小密度


// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC-DC




#endif