#ifndef WAVE_CONFIGURATION
#define WAVE_CONFIGURATION

#include "ns3/core-module.h"
#include "ns3/wave-module.h"
#include "ns3/network-module.h"

using namespace ns3;

/*
* 设置节点的网络设备
*   包括节点的netdevice和channel
*
*/
class waveConfiguration
{
    public:
        waveConfiguration ();
        virtual ~waveConfiguration ();

        NetDeviceContainer ConfigureDevices (NodeContainer &n);
};


#endif