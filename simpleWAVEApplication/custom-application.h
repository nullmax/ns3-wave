#ifndef WAVETEST_CUSTOM_APPLICATION_H
#define WAVETEST_CUSTOM_APPLICATION_H
#include "ns3/application.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include <vector>

using namespace ns3;
    
    typedef struct 
    {
        Address neighbor_mac;
        Time last_beacon;
    } NeighborInformation;

    class CustomApplication : public ns3::Application
    {
        public: 
            
            static TypeId GetTypeId (void);
            virtual TypeId GetInstanceTypeId (void) const;

            CustomApplication();
            ~CustomApplication();

            //向周围邻居节点广播
            void BroadcastInformation();

            //收到数据包后的回调
            bool ReceivePacket (Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender);

            //设置广播的间隔
            void SetBroadcastInterval (Time interval);

            //收到数据包后更新邻居节点
            void UpdateNeighbor (Address addr);
            
            //打印邻居节点
            void PrintNeighbors ();
            
            //设置wifi模式
            void SetWifiMode (WifiMode mode);

            //移除长时间未通信节点
            void RemoveOldNeighbors ();


        private:
            //StartApplication函数是应用启动后第一个调用的函数
            void StartApplication();
            Time m_broadcast_time; //广播的时间间隔
            uint32_t m_packetSize; //广播数据包的大小
            Ptr<WaveNetDevice> m_waveDevice; //车辆的WAVE设备  
            std::vector <NeighborInformation> m_neighbors; //节点的所有邻居节点
            Time m_time_limit; //移除超过m_time_limit未通信的节点
            WifiMode m_mode; //wifi的模式
    };

#endif