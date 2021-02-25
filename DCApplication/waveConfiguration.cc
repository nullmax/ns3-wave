#include "waveConfiguration.h"

waveConfiguration::waveConfiguration(){}
waveConfiguration::~waveConfiguration () {}

NetDeviceContainer waveConfiguration::ConfigureDevices (NodeContainer& nodes)
{
  /*
    Setting up WAVE devices. With PHY & MAC using default settings. 
  */
  //默认情况下
  //propagation delay equal to a constant, the speed of light
  //a propagation loss based on a log distance model with a reference loss of 46.6777 dB at reference distance of 1m.
  //YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();

  //设置通信信道
  YansWifiChannelHelper waveChannel;
  waveChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  waveChannel.AddPropagationLoss("ns3::RangePropagationLossModel"); //默认的通信范围为250m，默认情况下车身婧长度为5米，最小间距为2.5米，单车道最多33辆车
  
  //设置通信物理层
  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  wavePhy.Set ("TxPowerStart", DoubleValue (5) );
  wavePhy.Set ("TxPowerEnd", DoubleValue (33) );
  wavePhy.Set ("TxPowerLevels", UintegerValue (8));
  //设置通信mac层
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();

  waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
  						"DataMode", StringValue ("OfdmRate6MbpsBW10MHz"	),
  						"ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
  						"NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz"));

  NetDeviceContainer devices = waveHelper.Install (wavePhy, waveMac, nodes);

  //if you want PCAP trace, uncomment this!
  //wavePhy.EnablePcap ("custom-application", devices); //This generates *.pcap files

  return devices;
}