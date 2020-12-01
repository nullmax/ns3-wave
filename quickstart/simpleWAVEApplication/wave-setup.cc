#include "wave-setup.h"

using namespace ns3;


WaveSetup::WaveSetup(){}
WaveSetup::~WaveSetup () {}

NetDeviceContainer WaveSetup::ConfigureDevices (NodeContainer& nodes)
{
  YansWifiChannelHelper waveChannel = YansWifiChannelHelper::Default ();
  YansWavePhyHelper wavePhy =  YansWavePhyHelper::Default ();
  wavePhy.SetChannel (waveChannel.Create ());
  wavePhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  wavePhy.Set ("TxPowerStart", DoubleValue (5) );
  wavePhy.Set ("TxPowerEnd", DoubleValue (33) );
  wavePhy.Set ("TxPowerLevels", UintegerValue (8));
  QosWaveMacHelper waveMac = QosWaveMacHelper::Default ();
  WaveHelper waveHelper = WaveHelper::Default ();
  waveHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
  						"DataMode", StringValue ("OfdmRate6MbpsBW10MHz"	),
  						"ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
  						"NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz"));

  NetDeviceContainer devices = waveHelper.Install (wavePhy, waveMac, nodes);

  return devices;
}