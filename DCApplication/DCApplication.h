#ifndef __DC__APPLICATION__
#define __DC__APPLICATION__

#include <iostream>
#include <vector>
#include <string>

#include "ns3/application.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"

//guiyuan
#include "urbanEnvironment.h"
#include "customDataTag.h"

using namespace ns3;
using namespace std;

class DCApplication : public ns3::Application
{

public:

    static int head_count;
    static int head_sum;
    static int common_count;
    static int common_sum;

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;

    //构造函数
    DCApplication(); 
    //析构函数
    ~DCApplication();

    //设置m_mode
    void SetWifiMode (WifiMode mode);

    //设置m_packetSize
    void SetCommunicationPacketSize(uint32_t ps);

    //设置m_broadcast_interval
    void SetBroadcastInterval (Time interval);

    //设置m_neighbor_outofcontact_time_limit
    void SetMyNeighborUutofcontactTimeLimit(Time interval);

    //设置m_remove_neighbor_frequency
    void SetMyRemoveNeighborFrequency(Time frequency);

    //设置m_autonomousVehicleGroupRoles
    void SetAutonomousVehicleGroupRoles(int role);

    //设置m_leaderNode_outofcontact_time_limit
    void SetMyLeaderNodeOutofcontactTimeLimit(Time limit);

    //设置m_managedNode_outofcontact_time_limit
    void SetMyManagedNodeOutofcontactTimeLimit(Time limit);

    //设置m_heartBeat_interval
    // void SetMyHeartBeatInterval(Time interval);  

    //设置IN_TIME
    // void SetIN_TIME(Time t);

    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------  

    //节点收到数据包的回调
    bool ReceivePacket (Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender);

    //接收数据包后更新邻居无人驾驶车辆节点信息
    void UpdateNeighborAutonomousVehicleWhenReceiveAPacket(uint32_t nodeID, Address addr, Time beaconTime, Vector currentPosition,
        Vector currentVelocity, uint32_t role, uint32_t hop, uint32_t mnc, uint32_t nc);

    //节点收到请求加入车群请求后，开始处理车辆加入车群
    // void receiveJOINREQ(uint32_t nodeID, Address addr, Time beaconTime, Vector currentPosition,
    //     Vector currentVelocity);

    //节点发送请求后，收到回复
    void receiveJOINRESP(const Address & addr, CustomDataTag & tag);

    //接收到引领节点发送的心跳包后，更新引领节点的信息
    void UpdateMyLeaderNodeStatus(Address addr, Time beaconTime, Vector currentPosition, 
        Vector currentVelocity);

    //接收到被管理节点发送的心跳包后，更新被管理节点的信息
    void UpdateMyManagedNodeStatus(Address addr, Time beaconTime, Vector currentPosition,
        Vector currentVelocity, uint32_t itsmanagedNodeCounts);

    //父节点成为CH后，子节点更新状态
    // void MyfatherUpIAlsoUp();

    //------------------------------------

    //向1跳以内的邻居节点广播消息
    void BroadcastInformationtoOneHopNeighbor();

    //开始选择CH
    void SelectCoreNode();

    //计算m_following_degree和m_AvgRelM的加权和
    // void calculate_lnd();

    // //节点是SE状态，请求通过CH加入车群
    // void JoinaGroupThrouhCH();

    // //节点通过CH加入不成功，通过CM加入
    // void JoinAgainThrouhCM();

    // //节点判断是否自理山头
    // void JoinThird();

    //周期性移除长时间没有通信的节点
    void IterateRemoveOldones();

    //工具函数，向targetAddr发送messageType类型的消息
    bool SendMessageToaNode(uint32_t messageType, Address targetAddr);

    //引领节点离开后，次级引领节点晋升的函数
    // void IUpgrade();

    //计算车辆节点的运动方向
    void updateM_angle();

    //输出车群信息
    void PrintGroupInfo();

    //统计车群信息
    void becomealeader(Time t);
    void giveupleader(Time t);
    void becomeacommon(Time t);
    void giveupcommon(Time t);

    bool iAmLeader = false;
    int leadercounts=0;
    Time becomeleadertime = Seconds(0);
    Time leaderduration = Seconds(0);

    bool iAmCommon = false;
    int commoncounts=0;
    Time becomecommontime = Seconds(0);
    Time commonduration = Seconds(0);

private:
    //这个函数将在应用开始后被调用
    void StartApplication();
    //数据包的大小
    uint32_t m_packetSize;
    //wifi的模式
    WifiMode m_mode;
    //一个WaveNetDevice设备
    Ptr<WaveNetDevice> m_waveDevice;  

    //Beacon广播的频率
    Time m_broadcast_interval;
    //引领节点向普通节点、普通节点向普通节点、普通节点向引领节点发送心跳包的时间间隔，证明彼此还存活
    // Time m_heartBeat_interval; //已经迁移至IterateRemoveOldones中完成

    //邻居节点更新频率
    Time m_remove_neighbor_frequency;
    //邻居节点最长不通信时间，超过该时间将被移除
    Time m_neighbor_outofcontact_time_limit;
    //次级引领长时间未收到引领节点的消息，将晋升为引领节点，普通节点未收到次级引领节点的消息将重新请求加入车群
    Time m_leaderNode_outofcontact_time_limit;
    //引领节点或次级引领节点的管理节点的最长不通信时间，超过改时间，被管理节点将被移除
    Time m_managedNode_outofcontact_time_limit;

    //节点的邻居无人驾驶车辆节点信息
    std::list <NeighborAutonomousVehicleInfo> m_neighbors_autonomous;
    
    //无人驾驶车辆在车群中的角色
    int m_autonomousVehicleGroupRoles;
    //本无人驾驶车辆节点的引领节点，如果本无人驾驶车辆自身是引领节点，则引领节点就是自身
    NeighborAutonomousVehicleInfo m_leaderNode;
    //本无人驾驶车辆节点到引领节点的跳数，通过引领节点加入的游离节点到引领节点跳数为1，通过普通节点加入的游离节点到引领节点跳数为2
    uint32_t m_hopCountsToLeaderNode;
    //本无人驾驶车辆节点管理的普通节点，注意！普通节点需要定期向引领节点发送被管理节点的信息
    std::vector <NeighborAutonomousVehicleInfo> m_manangeAutonomousVehicles;

    //无人驾驶车辆节点的运动方向
    uint32_t m_angle;

    //车群形成所使用的数据包总量
    uint32_t dataPacketsCounts;
    
};


#endif