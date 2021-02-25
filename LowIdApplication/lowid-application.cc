#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "lowid-application.h"
#include "lowid-data-tag.h"

const uint32_t LowIdApplication::UNKNOWN_CID = -1;

const uint32_t LowIdApplication::MSG_BROADCAST = 0;
const uint32_t LowIdApplication::MSG_LOWID_ELECTION = 1;

uint32_t LowIdApplication::m_head_liveness_sum = 0;
uint32_t LowIdApplication::m_head_count = 0;

uint32_t LowIdApplication::m_common_liveness_sum = 0;
uint32_t LowIdApplication::m_common_count = 0;

uint32_t LowIdApplication::m_noise_liveness_sum = 0;

uint32_t LowIdApplication::m_msg_count = 0;

NS_LOG_COMPONENT_DEFINE ("LowIdApplication");
NS_OBJECT_ENSURE_REGISTERED(LowIdApplication);

TypeId LowIdApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LowIdApplication")
                .SetParent <Application> ()
                .AddConstructor<LowIdApplication> ()
                .AddAttribute ("Interval", "Broadcast Interval",
                      TimeValue (MilliSeconds(100)),
                      MakeTimeAccessor (&LowIdApplication::m_broadcast_time),
                      MakeTimeChecker()
                      )
                ;
    return tid;
}

TypeId LowIdApplication::GetInstanceTypeId() const
{
    return LowIdApplication::GetTypeId();
}

LowIdApplication::LowIdApplication()
{
    m_broadcast_time = MilliSeconds (100); 
    m_packetSize = 1000;
    m_time_limit = Seconds (5);
    m_mode = WifiMode("OfdmRate6MbpsBW10MHz");

    m_recalc_secore = true;

    m_head_liveness = 0;
    m_common_liveness = 0;
    m_noise_liveness = 0;
}

LowIdApplication::~LowIdApplication()
{
    m_head_liveness_sum += m_head_liveness;
    m_common_liveness_sum += m_common_liveness;
    m_noise_liveness_sum += m_noise_liveness;
}

void LowIdApplication::StartApplication()
{
    Ptr<Node> n = GetNode ();

    m_nodeId = n->GetId();
    srand(m_nodeId);
    int rand_tmp = rand() % 10;
    m_ell = rand_tmp < 3 ? 1 : (rand_tmp < 7 ? 2 : 3);

    rand_tmp = rand() % 10;
    CVSSMetricRL rl = rand_tmp < 3 ? CVSSMetricRL::TemporaryFix : CVSSMetricRL::OfficialFix;
    rand_tmp = rand() % 10;
    CVSSMetricUI mui = rand_tmp < 3 ? CVSSMetricUI::None : CVSSMetricUI::Required;
    rand_tmp = rand() % 10;
    CVSSMetricPRCIA mc = rand_tmp < 9 ? CVSSMetricPRCIA::Low : CVSSMetricPRCIA::High;
    rand_tmp = rand() % 10;
    CVSSMetricPRCIA mi = rand_tmp < 7 ? CVSSMetricPRCIA::Low : CVSSMetricPRCIA::High;
    rand_tmp = rand() % 10;
    CVSSMetricPRCIA ma = rand_tmp < 7 ? CVSSMetricPRCIA::Low : CVSSMetricPRCIA::High;
    m_cvss = CVSS(CVSSMetricAV::Adjacent, CVSSMetricAC::High, CVSSMetricPRCIA::High, CVSSMetricUI::Required,
                                CVSSMetricS::Changed, CVSSMetricPRCIA::Low, CVSSMetricPRCIA::Low, CVSSMetricPRCIA::Low,
                                CVSSMetricE::Unproven, rl, CVSSMetricRC::NotDefined,
                                CVSSMetricCIAR::High, CVSSMetricCIAR::High, CVSSMetricCIAR::Medium,
                                CVSSMetricAV::Local, CVSSMetricAC::Low, CVSSMetricPRCIA::High, mui,
                                CVSSMetricS::Changed, mc, mi, ma);
    m_clusterId = UNKNOWN_CID;

    NeighborInformation new_n;
    new_n.last_beacon = Now ();
    new_n.nodeId = m_nodeId;
    new_n.clusterId = m_clusterId;
    m_GAMMA.push_back (new_n);
   
    for (uint32_t i = 0; i < n->GetNDevices (); i++)
    {
        Ptr<NetDevice> dev = n->GetDevice (i);
        if (dev->GetInstanceTypeId () == WaveNetDevice::GetTypeId())
        {
            //获取节点的wave设备
            m_waveDevice = DynamicCast <WaveNetDevice> (dev);
            //接收数据包的回调
            dev->SetReceiveCallback (MakeCallback (&LowIdApplication::ReceivePacket, this));

            break;
        } 
    }
    if (m_waveDevice)
    {
        //设置随机数
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        Time random_offset = MicroSeconds (rand->GetValue(0,1000));
        //m_broadcast_time秒钟后调用函数BroadcastInformation
        Simulator::Schedule (m_broadcast_time+random_offset, &LowIdApplication::BroadcastInformation, this);

        Simulator::Schedule (Seconds(4.5)+random_offset, &LowIdApplication::StartElection, this);
    }
    else
    {
        NS_FATAL_ERROR ("There's no WaveNetDevice in your node");
    }
    //周期性检查邻居节点，并移除长时间未通信的节点
    Simulator::Schedule (Seconds (1), &LowIdApplication::RemoveOldNeighbors, this);

    Simulator::Schedule (Seconds (10), &LowIdApplication::ResetCID, this);
    // NS_LOG_INFO("App: " << m_nodeId);
}

double LowIdApplication::CalcScore()
{   
    if (m_recalc_secore)
    {
        double scores[3];
        m_cvss.CalcScore(scores);
        m_score = 10/(m_ell * log(scores[0]+scores[1]+scores[2]));
        m_recalc_secore = false;
    }
    return m_score;
}

void LowIdApplication::SetBroadcastInterval (Time interval)
{
    m_broadcast_time = interval;
}

void LowIdApplication::SetWifiMode (WifiMode mode)
{
    m_mode = mode;
}

uint32_t LowIdApplication::MinimailNodeID()
{
    uint32_t res = -1;
    for (std::vector<NeighborInformation>::iterator it = m_GAMMA.begin(); it != m_GAMMA.end(); it++ )
    {
        if (it->nodeId < res)
        {
            res = it->nodeId;
        }
    }
    return res;
}

void LowIdApplication::RemoveIDInGAMMA(uint32_t id)
{
    for (std::vector<NeighborInformation>::iterator it = m_GAMMA.begin(); it != m_GAMMA.end();)
    {
        if (it->nodeId == id)
        {
            it = m_GAMMA.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LowIdApplication::SendMessage(const Address & addr, const uint32_t msg_type)
{
    ++m_msg_count;
    //数据包参数
    TxInfo tx;
    tx.channelNumber = CCH; 
    tx.priority = 7;
    tx.txPowerLevel = 7;
    tx.dataRate = m_mode;
    tx.preamble = WIFI_PREAMBLE_LONG;
    
    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    //tag中携带本节点的ID、位置信息、发送时间信息
    LowIdDataTag tag;
    tag.SetMessageType(msg_type);
    tag.SetNodeId (m_nodeId);
    tag.SetCID(m_clusterId);
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    tag.SetScore(m_score_new);

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, addr, 0x88dc, tx);
}

void LowIdApplication::BroadcastToGAMMA()
{
    for (std::vector<NeighborInformation>::iterator it = m_GAMMA.begin(); it != m_GAMMA.end(); it++ )
    {
        if(it->nodeId != m_nodeId)
        {
            SendMessage(it->neighbor_mac, MSG_LOWID_ELECTION);
        }
    }
}

void LowIdApplication::StartElection()
{
    if(m_nodeId == MinimailNodeID())
    {
        m_clusterId = m_nodeId;
        ++m_head_count;
        RemoveIDInGAMMA(m_nodeId);
        BroadcastToGAMMA();
    }
}

void LowIdApplication::BroadcastInformation()
{   
    SendMessage(Mac48Address::GetBroadcast(), MSG_BROADCAST);

    //每m_broadcast_time广播一次
    Simulator::Schedule (m_broadcast_time, &LowIdApplication::BroadcastInformation, this);
}

bool LowIdApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    LowIdDataTag tag;
    bool useTag = packet->PeekPacketTag (tag); 
    if (useTag)
    {   
        uint32_t id = tag.GetNodeId();
        uint32_t cid = tag.GetCID();
        // NS_LOG_INFO ("ID: " << m_nodeId << "\t消息类型: " << tag.GetMessageType() << "\t消息发送方的ID: " << id << "\t消息发送方的CID: " << cid << "\t消息发送时间: " << tag.GetTimestamp() << "\t消息延迟="<< Now()-tag.GetTimestamp());

        switch (tag.GetMessageType())
        {
        case MSG_LOWID_ELECTION:
            if((cid == id) && (m_clusterId == UNKNOWN_CID || m_clusterId > cid))
            {   
                ++m_common_count;
                m_clusterId = cid;
            }
            
            RemoveIDInGAMMA(id);

            if(m_nodeId == MinimailNodeID() )
            {
                if(m_clusterId == UNKNOWN_CID)
                {   
                    ++m_head_count;
                    m_clusterId = m_nodeId;
                }
                BroadcastToGAMMA();
                RemoveIDInGAMMA(m_nodeId);
            }
           break;
        default:
            break;
        }
    }
    
    UpdateNeighbor (sender, useTag, tag);

    return true;
}

void LowIdApplication::UpdateNeighbor (Address addr, bool useTag, LowIdDataTag & tag)
{
    if(!useTag || (useTag && tag.GetMessageType() == MSG_LOWID_ELECTION))
    {
        return;
    }

    bool found = false;
    
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        if (it->neighbor_mac == addr)
        {
            it->last_beacon = Now();
            found = true;
            if(useTag)
            {
                it->nodeId = tag.GetNodeId();
                it->clusterId = tag.GetCID();
                it->m_score = tag.GetScore();
            }
            break;
        }
    }
    if (!found)
    {
        NeighborInformation new_n;
        new_n.neighbor_mac = addr;
        new_n.last_beacon = Now ();
        if(useTag)
        {
            new_n.nodeId = tag.GetNodeId();
            new_n.clusterId = tag.GetCID();
            new_n.m_score = tag.GetScore();
        }
        m_neighbors.push_back (new_n);
    }

    bool GAMMA_found = false;
    for (std::vector<NeighborInformation>::iterator it = m_GAMMA.begin(); it != m_GAMMA.end(); it++ )
    {
        if (it->neighbor_mac == addr)
        {
            it->last_beacon = Now();
            GAMMA_found = true;
            if(useTag)
            {
                it->nodeId = tag.GetNodeId();
                it->clusterId = tag.GetCID();
            }
            break;
        }
    }
    if (!found && !GAMMA_found)
    {
        if(useTag && tag.GetCID() == UNKNOWN_CID)
        {
            NeighborInformation new_n;
            new_n.neighbor_mac = addr;
            new_n.last_beacon = Now ();
            new_n.nodeId = tag.GetNodeId();
            new_n.clusterId = tag.GetCID();
            m_GAMMA.push_back (new_n);
            if(m_nodeId < tag.GetNodeId())
            {
                if(m_clusterId == MinimailNodeID())
                {
                    m_clusterId = m_nodeId;
                }
                SendMessage(addr, MSG_LOWID_ELECTION);
            }
        }

   }
}

void LowIdApplication::PrintNeighbors ()
{
    NS_LOG_INFO("Node: " << m_nodeId << " CID: " << m_clusterId << " Neighbor Size: " << m_neighbors.size());
    // for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    // {
    //     std::cout << "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon << std::endl;
    // }
    // std::cout <<std::endl;
}

void LowIdApplication::RemoveOldNeighbors ()
{
    m_score_new = CalcScore();
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end();)
    {
        //获取最后通信时间到现在的时间
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            if(it->nodeId == m_clusterId)
            {
                m_clusterId = UNKNOWN_CID;
            }
            it = m_neighbors.erase (it);
            break;
        }
        else
        {   
            if(m_nodeId == m_clusterId)
            {
                m_score_new += it->m_score;
            }
            ++it;
        }
    }
    
    if(m_nodeId == m_clusterId)
    {
        m_score_new /= m_neighbors.size() + 1;
    }

    for (std::vector<NeighborInformation>::iterator it = m_GAMMA.begin(); it != m_GAMMA.end();)
    {
        //获取最后通信时间到现在的时间
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            if(it->nodeId == m_clusterId)
            {
                m_clusterId = UNKNOWN_CID;
            }
            it = m_GAMMA.erase (it);
            break;
        }
        else
        {
            ++it;
        }
    }
    //周期性检查并移除长时间未通信节点

    if(m_nodeId == m_clusterId)
    {
        ++m_head_liveness;
        NS_LOG_INFO(Now().GetSeconds() << " " << m_nodeId << " " << m_score_new);
    }
    else if(m_clusterId != UNKNOWN_CID)
    {
        ++m_common_liveness;
    }
    else
    {
        ++m_noise_liveness;
    }
    
    Simulator::Schedule (Seconds (1), &LowIdApplication::RemoveOldNeighbors, this);
}

void LowIdApplication::ResetCID ()
{
    bool reset = true;
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); ++it)
    {
        if (it->clusterId == m_clusterId) 
        {
            reset = false;
        }
    }
    if(reset)
    {
        m_clusterId = UNKNOWN_CID;
    }
    Simulator::Schedule (Seconds (5), &LowIdApplication::ResetCID, this);
}
 