#ifndef WAVETEST_CUSTOM_APPLICATION_H
#define WAVETEST_CUSTOM_APPLICATION_H
#include "ns3/application.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include "cvss.h"
#include "as-data-tag.h"
#include <list>

using namespace ns3;
    
    typedef struct 
    {
        Address neighbor_mac;
        Time last_beacon;
        uint32_t m_nodeId; //节点的ID
        Vector m_currentPosition; //节点的位置
        double m_CVSS_score; // 节点安全评分
    } NeighborInformation;

    class ASApplication : public ns3::Application
    {
        public: 
            
            static TypeId GetTypeId (void);
            virtual TypeId GetInstanceTypeId (void) const;

            ASApplication();
            ~ASApplication();

            //向周围邻居节点广播
            void BroadcastInformation();

            //收到数据包后的回调
            bool ReceivePacket (Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender);

            //设置广播的间隔
            void SetBroadcastInterval (Time interval);

            //收到数据包后更新邻居节点
            void UpdateNeighbor (Address addr, ASDataTag tag, bool useTag);

            //收到数据包后更新下属节点
            void UpdateSubordinates (Address addr, ASDataTag tag, bool useTag);
            
            //打印邻居节点
            void PrintNeighbors ();
            
            //设置wifi模式
            void SetWifiMode (WifiMode mode);

            //移除长时间未通信节点
            void RemoveOldNeighbors ();


        private:
            //StartApplication函数是应用启动后第一个调用的函数
            void StartApplication();

            // 计算安全得分
            double CalcScore();

            void StartElection();
            void SendHeartBeat();
            void SetTxInfo(TxInfo & p_tx);
            void SetASDataTag(ASDataTag & tag, uint32_t type);
            void SendMessage(uint32_t type, const Address & addr);
            
            Time m_broadcast_time; //广播的时间间隔
            uint32_t m_packetSize; //广播数据包的大小
            Ptr<WaveNetDevice> m_waveDevice; //车辆的WAVE设备  
            std::list <NeighborInformation> m_neighbors; //节点的所有邻居节点
            std::list <NeighborInformation> m_subordinates; //节点的下属节点
            Time m_time_limit; //移除超过m_time_limit未通信的节点
            WifiMode m_mode; //wifi的模式
            CVSS m_cvss; // CVSS计算模块
            uint32_t m_role; //车群角色
            uint32_t m_vote_count; // 投票计数
            uint32_t m_vote_failure_count; // 竞选失败计数
            uint32_t m_app_id;  //

            static const int minPts = 4;
            static const int minVotes = 2;
            static uint32_t app_count;

            static const uint32_t NOISE = 0;
            static const uint32_t BORDER = 1;
            static const uint32_t CORE = 2;
            static const uint32_t SECONDARY = 3;
            static const uint32_t HEAD = 4;
    };

#endif