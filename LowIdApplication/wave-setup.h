#ifndef WAVE_SETUP_H
#define WAVE_SETUP_H
#include "ns3/core-module.h"
#include "ns3/wave-module.h"
#include "ns3/network-module.h"

namespace ns3
{
  class WaveSetup
  {
    public:
      WaveSetup ();
      virtual ~WaveSetup ();
      //配置WAVE设备
      NetDeviceContainer ConfigureDevices (NodeContainer &n);
  };
}

#endif 