#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"


//DC算法应用
#include "DCApplication.h"

#include "waveConfiguration.h"
#include "urbanEnvironment.h"


using namespace ns3;
// using namespace std;


// static void
// CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility)
// {
//   Vector pos = mobility->GetPosition (); // Get position
//   Vector vel = mobility->GetVelocity (); // Get velocity

//   // Prints position and velocities
//   *os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y
//       << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
//       << ", z=" << vel.z << std::endl;
// }

NS_LOG_COMPONENT_DEFINE ("DCApplicationExample");

const double eps = 0.001;

int main(int argc, char *argv[])
{
    LogComponentEnable("DCApplicationExample", LOG_LEVEL_INFO);
    LogComponentEnable ("DCApplication", LOG_LEVEL_INFO);

    std::string m_traceFile = "/home/mak/Code/ns3-wave/cross/mobility10.tcl";
    uint32_t nNodes = 60;//节点数目
    double startTime = 0; //仿真时间
    double simTime = 60; //仿真时间
    double interval = 1; //广播的时间间隔

    CommandLine cmd;
    cmd.AddValue("nNodes", "Number of vehicle nodes", nNodes);
    cmd.AddValue("startTime", "Time of the simulation start", startTime);
    cmd.AddValue("simTime", "Time of the simulation", simTime);
    cmd.AddValue("interval", "Time interval of broadcast", interval);
    cmd.AddValue("traceFile", "Mobility trace file", m_traceFile);
    cmd.Parse(argc, argv);

    nNodes = nNodes == 0 ? 5 : nNodes;
    simTime = simTime < eps ? 60 : simTime;
    startTime = startTime > simTime ? 0 : startTime;
    interval = interval < eps ? 1 : interval;

    NS_LOG_INFO ("--nNodes=" << nNodes << " --startTime=" << startTime << " --simTime=" << simTime << " --interval=" << interval << " --m_traceFile=" << m_traceFile);

    //(1):创建节点
    NodeContainer nodes;
    nodes.Create(nNodes);

    //(2):创建节点之间的链路和网络设备
    waveConfiguration wC;
    NetDeviceContainer devices = wC.ConfigureDevices(nodes);
    
    //(3):配置节点的移动性
    Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
    ns2.Install ();

    //(4):节点位置移动的回调函数
    //创建输出流，打开log文件
    // std::ofstream os;
    // os.open (MOBILITYMONITORLOGFILE);//log文件的路径
    // //配置节点移动的回调
    // Config::Connect ("/NodeList/0/$ns3::MobilityModel/CourseChange",
    //                MakeBoundCallback (&CourseChange, &os));

    //(5):为节点创建应用程序
        //2020 PMC车群聚类方法
        for (uint32_t i=0; i<nodes.GetN(); i++)
        {
            //无人驾驶车辆节点
            Ptr<DCApplication> app_i = CreateObject<DCApplication>();
            app_i->SetWifiMode(WifiMode("OfdmRate6MbpsBW10MHz"));
            app_i->SetCommunicationPacketSize(32); //广播数据包的大小
            app_i->SetBroadcastInterval (Seconds(1.0)); //设置广播消息的发送间隔
            app_i->SetMyNeighborUutofcontactTimeLimit(Seconds (5)); //设置邻居节点最长不通信时间
            app_i->SetMyRemoveNeighborFrequency(Seconds(1.0)); //设置检查邻居的时间间隔
            app_i->SetAutonomousVehicleGroupRoles(FREE);//无人驾驶车辆初始状态下为初始化状态
            app_i->SetMyLeaderNodeOutofcontactTimeLimit(Seconds (5)); //设置父节点最长不通信时间
            app_i->SetMyManagedNodeOutofcontactTimeLimit(Seconds (5)); //设置被管理节点的最长不通信时间

            app_i->SetStartTime (Seconds (startTime)); //设置应用程序开始的时间
            app_i->SetStopTime (Seconds (simTime)); //设置应用程序结束的时间
            nodes.Get(i)->AddApplication(app_i); //为节点添加应用程序 
        }
    

    //(6):仿真开始->结束
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    //os.close (); //关闭log文件
    // std::cout<<"simulation end!"<<std::endl;//仿真结束

    printf("DC sum cout avg\n");
    printf("Head %d %d %d\n", DCApplication::head_sum, DCApplication::head_count, DCApplication::head_sum/DCApplication::head_count);
    printf("Common %d %d %d\n", DCApplication::common_sum, DCApplication::common_count, DCApplication::common_sum/DCApplication::common_count);
    return 0;
}