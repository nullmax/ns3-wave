#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "custom-application.h"
#include "wave-setup.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CustomApplicationExample");

void SomeEvent ()
{
  for (uint32_t i=0 ; i<NodeList::GetNNodes(); i++)
  {
    Ptr<Node> n = NodeList::GetNode(i);
    Ptr<CustomApplication> c_app = DynamicCast <CustomApplication> (n->GetApplication(0));
    c_app->SetWifiMode (WifiMode("OfdmRate3MbpsBW10MHz"));
  }
}

int main (int argc, char *argv[])
{
  CommandLine cmd;

  uint32_t nNodes = 5;//节点数目
  double simTime = 60; //仿真时间
  double interval = 0.5; //广播的时间间隔

  NodeContainer nodes;
  nodes.Create(nNodes);

  LogComponentEnable ("CustomApplication", LOG_LEVEL_FUNCTION);
  
  //使用NS3的移动模型，可以修改为SUMO的FCD输出
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install(nodes);
  for (uint32_t i=0 ; i<nodes.GetN(); i++)
  {
    Ptr<ConstantVelocityMobilityModel> cvmm = DynamicCast<ConstantVelocityMobilityModel> (nodes.Get(i)->GetObject<MobilityModel>());
    cvmm->SetPosition ( Vector (20+i*5, 20+(i%2)*5, 0));
    cvmm->SetVelocity ( Vector (10+((i+1)%2)*5,0,0) );
  }
 
  //配置WAVE设备
  WaveSetup wave;
  NetDeviceContainer devices = wave.ConfigureDevices(nodes);

  //为节点添加应用
  for (uint32_t i=0; i<nodes.GetN(); i++)
  {
    Ptr<CustomApplication> app_i = CreateObject<CustomApplication>();
    app_i->SetBroadcastInterval (Seconds(interval));
    app_i->SetStartTime (Seconds (0));
    app_i->SetStopTime (Seconds (simTime));
    nodes.Get(i)->AddApplication (app_i);
  }

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  std::cout << "Post Simulation: " << std::endl;
  
  for (uint32_t i=0 ; i<nodes.GetN(); i++)
  {
    Ptr<CustomApplication> appI = DynamicCast<CustomApplication> (nodes.Get(i)->GetApplication(0));
    appI->PrintNeighbors ();
  }

  Simulator::Destroy();

}
