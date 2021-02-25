#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/netanim-module.h"
#include "as-application.h"
#include "wave-setup.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ASApplicationExample");

void SomeEvent ()
{
  for (uint32_t i=0 ; i<NodeList::GetNNodes(); i++)
  {
    Ptr<Node> n = NodeList::GetNode(i);
    Ptr<ASApplication> c_app = DynamicCast <ASApplication> (n->GetApplication(0));
    c_app->SetWifiMode (WifiMode("OfdmRate3MbpsBW10MHz"));
  }
}

const double eps = 0.001;

int main (int argc, char *argv[])
{

  LogComponentEnable("ASApplicationExample", LOG_LEVEL_INFO);
  LogComponentEnable("ASApplication", LOG_LEVEL_INFO);

  std::string m_traceFile = "/mnt/d/Code/ns3-wave/cross/mobility10.tcl";
  uint32_t nNodes = 200;//节点数目
  double startTime = 550; //仿真时间
  double simTime = 750; //仿真时间
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

  NodeContainer nodes;
  nodes.Create(nNodes);
  NS_LOG_INFO ("Nodes created");

  Ns2MobilityHelper ns2m = Ns2MobilityHelper (m_traceFile);
  ns2m.Install (); // configure movements for each node, while reading trace file
  NS_LOG_INFO ("Mobility model installed");

  //使用NS3的移动模型
  // MobilityHelper mobility;
  // mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  // mobility.Install(nodes);
  // for (uint32_t i=0 ; i<nodes.GetN(); i++)
  // {
  //   Ptr<ConstantVelocityMobilityModel> cvmm = DynamicCast<ConstantVelocityMobilityModel> (nodes.Get(i)->GetObject<MobilityModel>());
  //   cvmm->SetPosition ( Vector (20+i*5, 20+(i%2)*5, 0));
  //   cvmm->SetVelocity ( Vector (10+((i+1)%2)*5,0,0) );
  // }
 
  //配置WAVE设备
  WaveSetup wave;
  NetDeviceContainer devices = wave.ConfigureDevices(nodes);
  NS_LOG_INFO("WAVE device installed");

  //为节点添加应用
  for (uint32_t i=0; i<nodes.GetN(); i++)
  {
    Ptr<ASApplication> app_i = CreateObject<ASApplication>();
    app_i->SetBroadcastInterval (Seconds(interval));
    // app_i->SetStartTime (Seconds (ns2_utility.GetEntryTimeForNode(i)));
    app_i->SetStartTime (Seconds (startTime));
    app_i->SetStopTime (Seconds(simTime));
    // app_i->SetStopTime (Seconds (simTime));
    nodes.Get(i)->AddApplication (app_i);
  }
  NS_LOG_INFO ("Application installed");

  Simulator::Stop(Seconds(simTime));

  // AnimationInterface anim("as-mobility.xml");

  Simulator::Run();
  
  NS_LOG_INFO ("Post Simulation: ");
  // std::cout << "Post Simulation: " << std::endl;
  
  for (uint32_t i=0 ; i<nodes.GetN(); i++)
  {
    Ptr<ASApplication> appI = DynamicCast<ASApplication> (nodes.Get(i)->GetApplication(0));
    appI->PrintNeighbors ();
  }

  Simulator::Destroy();

  printf("AS sum cout avg\n");
  printf("Head %d %d %d\n", ASApplication::m_head_liveness_sum, ASApplication::m_head_count, ASApplication::m_head_liveness_sum/ASApplication::m_head_count);
  printf("Core %d %d %d\n", ASApplication::m_core_liveness_sum, ASApplication::m_core_count, ASApplication::m_core_liveness_sum/ASApplication::m_core_count);
  printf("Border %d %d %d\n", ASApplication::m_border_liveness_sum, ASApplication::m_border_count, ASApplication::m_border_liveness_sum/ASApplication::m_border_count);
  printf("Noise %d\n", ASApplication::m_noise_liveness_sum);
  printf("MSG Count %d\n", ASApplication::m_msg_count);
}
