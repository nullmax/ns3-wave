#ifndef WAVETEST_CUSTOM_APPLICATION_H
#define WAVETEST_CUSTOM_APPLICATION_H
#include "ns3/application.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include "lowid-data-tag.h"
#include "cvss.h"
#include <vector>

using namespace ns3;
    
    typedef struct 
    {
        Address neighbor_mac;
        Time last_beacon;
        uint32_t nodeId;
        uint32_t clusterId;
        double m_score;
    } NeighborInformation;

    class LowIdApplication : public ns3::Application
    {
        public: 
            
            static uint32_t m_head_liveness_sum;
            static uint32_t m_head_count;

            static uint32_t m_common_liveness_sum;
            static uint32_t m_common_count;

            static uint32_t m_noise_liveness_sum;

            static uint32_t m_msg_count;

            static TypeId GetTypeId (void);
            virtual TypeId GetInstanceTypeId (void) const;

            LowIdApplication();
            ~LowIdApplication();

            //向周围邻居节点广播
            void BroadcastInformation();

            void BroadcastToGAMMA();

            void StartElection();

            void SendMessage(const Address & addr, const uint32_t msg_type);

            //收到数据包后的回调
            bool ReceivePacket (Ptr<NetDevice> device,Ptr<const Packet> packet,uint16_t protocol, const Address &sender);

            //设置广播的间隔
            void SetBroadcastInterval (Time interval);

            //收到数据包后更新邻居节点
            void UpdateNeighbor (Address addr, bool useTag, LowIdDataTag & tag);

             // 计算安全得分
            double CalcScore();

            //打印邻居节点
            void PrintNeighbors ();
            
            //设置wifi模式
            void SetWifiMode (WifiMode mode);

            //移除长时间未通信节点
            void RemoveOldNeighbors ();

            uint32_t MinimailNodeID();

            void RemoveIDInGAMMA(uint32_t id);
            
            void ResetCID ();

        private:
            //StartApplication函数是应用启动后第一个调用的函数
            void StartApplication();
            Time m_broadcast_time; //广播的时间间隔
            uint32_t m_packetSize; //广播数据包的大小
            Ptr<WaveNetDevice> m_waveDevice; //车辆的WAVE设备  
            std::vector <NeighborInformation> m_neighbors; //节点的所有邻居节点
            std::vector <NeighborInformation> m_GAMMA; //节点的所有邻居节点
            Time m_time_limit; //移除超过m_time_limit未通信的节点
            WifiMode m_mode; //wifi的模式
            CVSS m_cvss; // CVSS计算模块
            bool m_recalc_secore; //
            double m_score; // total score;
            double m_score_new; // avg total score;

            uint32_t m_nodeId;
            uint32_t m_clusterId;

            uint32_t m_ell; //预估损失等级

            uint32_t m_head_liveness;
            uint32_t m_common_liveness;
            uint32_t m_noise_liveness;


            static const uint32_t UNKNOWN_CID;

            static const uint32_t MSG_BROADCAST;
            static const uint32_t MSG_LOWID_ELECTION;
    };

#endif