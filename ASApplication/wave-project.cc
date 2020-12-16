#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "as-application.h"
#include "wave-setup.h"
#include "ns2-node-utility.h"

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
  std::string m_traceFile = "/home/mak/Code/ns3-wave/cross/mobility.tcl";

  LogComponentEnable("ASApplicationExample", LOG_LEVEL_INFO);
  LogComponentEnable("ASApplication", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Parsing mobility file");

  Ns2NodeUtility ns2_utility (m_traceFile);

  uint32_t nNodes = ns2_utility.GetNNodes();//节点数目
  double simTime = ns2_utility.GetSimulationTime(); //仿真时间
  // uint32_t nNodes = 60;//节点数目
  // double simTime = 60; //仿真时间
  double interval = 1; //广播的时间间隔

  CommandLine cmd;
  cmd.AddValue("nNodes", "Number of vehicle nodes", nNodes);
  cmd.AddValue("simTime", "Time of the simulation", simTime);
  cmd.AddValue("interval", "Time interval of broadcast", interval);
  cmd.AddValue("traceFile", "Mobility trace file", m_traceFile);
  cmd.Parse(argc, argv);

  nNodes = nNodes == 0 ? 5 : nNodes;
  simTime = simTime < eps ? 60 : simTime;
  interval = interval < eps ? 1 : interval;
  

  NS_LOG_INFO ("nNodes: " << nNodes << " simTime: " << simTime << " interval: " << interval);

  NS_LOG_INFO ("Creating Nodes");
  NodeContainer nodes;
  nodes.Create(nNodes);
  NS_LOG_INFO ("Nodes created");

  LogComponentEnable ("ASApplication", LOG_LEVEL_FUNCTION);
  Ns2MobilityHelper ns2m = Ns2MobilityHelper (m_traceFile);
  ns2m.Install (); // configure movements for each node, while reading trace file
  NS_LOG_INFO ("Mobility model installed");

  //使用NS3的移动模型，可以修改为SUMO的FCD输出
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
    // app_i->SetStopTime (Seconds (ns2_utility.GetExitTimeForNode(i)));
    app_i->SetStartTime (Seconds (110));
    app_i->SetStopTime (Seconds (simTime));
    nodes.Get(i)->AddApplication (app_i);
  }
  NS_LOG_INFO ("Application installed");

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  
  NS_LOG_INFO ("Post Simulation: ");
  // std::cout << "Post Simulation: " << std::endl;
  
  for (uint32_t i=0 ; i<nodes.GetN(); i++)
  {
    Ptr<ASApplication> appI = DynamicCast<ASApplication> (nodes.Get(i)->GetApplication(0));
    appI->PrintNeighbors ();
  }

  Simulator::Destroy();

}
