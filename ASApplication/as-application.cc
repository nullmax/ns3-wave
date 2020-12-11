#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "as-application.h"

uint32_t ASApplication::app_count = 0;

NS_LOG_COMPONENT_DEFINE("ASApplication");
NS_OBJECT_ENSURE_REGISTERED(ASApplication);

TypeId ASApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ASApplication")
                .SetParent <Application> ()
                .AddConstructor<ASApplication> ()
                .AddAttribute ("Interval", "Broadcast Interval",
                      TimeValue (MilliSeconds(100)),
                      MakeTimeAccessor (&ASApplication::m_broadcast_time),
                      MakeTimeChecker()
                      )
                ;
    return tid;
}

TypeId ASApplication::GetInstanceTypeId() const
{
    return ASApplication::GetTypeId();
}

ASApplication::ASApplication()
{
    m_app_id = app_count++;
    NS_LOG_INFO("APP" << m_app_id << " started");
    m_broadcast_time = MilliSeconds (1000); 
    m_packetSize = 1000;
    m_time_limit = Seconds (5);
    m_mode = WifiMode("OfdmRate6MbpsBW10MHz");
    m_role = NOISE;
    m_vote_count = 0;
    m_vote_failure_count = 0;
    m_cvss = CVSS(CVSSMetricAV::Local, CVSSMetricAC::Low, CVSSMetricPRCIA::High, CVSSMetricUI::Required,
                                CVSSMetricS::Unchanged, CVSSMetricPRCIA::High, CVSSMetricPRCIA::None, CVSSMetricPRCIA::High,
                                CVSSMetricE::High, CVSSMetricRL::Workaround, CVSSMetricRC::Unknown,
                                CVSSMetricCIAR::High, CVSSMetricCIAR::High, CVSSMetricCIAR::Medium,
                                CVSSMetricAV::Local, CVSSMetricAC::Low, CVSSMetricPRCIA::High, CVSSMetricUI::None,
                                CVSSMetricS::Changed, CVSSMetricPRCIA::None, CVSSMetricPRCIA::High, CVSSMetricPRCIA::Low);
}

ASApplication::~ASApplication()
{

}

void ASApplication::StartApplication()
{   
    Ptr<Node> n = GetNode ();
    
    for (uint32_t i = 0; i < n->GetNDevices (); i++)
    {
        Ptr<NetDevice> dev = n->GetDevice (i);
        if (dev->GetInstanceTypeId () == WaveNetDevice::GetTypeId())
        {
            //获取节点的wave设备
            m_waveDevice = DynamicCast <WaveNetDevice> (dev);
            //接收数据包的回调
            dev->SetReceiveCallback (MakeCallback (&ASApplication::ReceivePacket, this));

            break;
        } 
    }
    if (m_waveDevice)
    {
        //设置随机数
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        Time offset = MicroSeconds (m_app_id * 3);
        //m_broadcast_time秒钟后调用函数BroadcastInformation
        Simulator::Schedule (m_broadcast_time + offset, &ASApplication::BroadcastInformation, this);

        Simulator::Schedule (Seconds(4.5) + offset, &ASApplication::StartElection, this);
    }
    else
    {
        NS_FATAL_ERROR ("There's no WaveNetDevice in your node");
    }
    //周期性检查邻居节点，并移除长时间未通信的节点
    Simulator::Schedule (Seconds (1), &ASApplication::RemoveOldNeighbors, this);
}


double ASApplication::CalcScore()
{
    double scores[3];
    m_cvss.CalcScore(scores);
    return scores[0]+scores[1]+scores[2];
}

void ASApplication::SetBroadcastInterval (Time interval)
{
    m_broadcast_time = interval;
}

void ASApplication::SetWifiMode (WifiMode mode)
{
    m_mode = mode;
}


void ASApplication::UpdateNeighbor (Address addr, ASDataTag tag, bool useTag)
{
    bool found = false;
    
    for (std::list<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        if (it->neighbor_mac == addr)
        {
            it->last_beacon = Now();
            found = true;
            if (useTag)
            {
                it->m_currentPosition = tag.GetPosition();
                it->m_CVSS_score = tag.GetScore();
                it->m_nodeId = tag.GetNodeId();
            }
            break;
        }
    }
    if (!found)
    {
        NeighborInformation new_n;
        new_n.neighbor_mac = addr;
        new_n.last_beacon = Now ();
        if (useTag)
        {
            new_n.m_currentPosition = tag.GetPosition();
            new_n.m_CVSS_score = tag.GetScore();
            new_n.m_nodeId = tag.GetNodeId();
        }
        m_neighbors.push_back (new_n);
    }
}

void ASApplication::UpdateSubordinates (Address addr, ASDataTag tag, bool useTag)
{
    bool found = false;
    
    for (std::list<NeighborInformation>::iterator it = m_subordinates.begin(); it != m_subordinates.end(); it++ )
    {
        if (it->neighbor_mac == addr)
        {
            it->last_beacon = Now();
            found = true;
            if (useTag)
            {
                it->m_currentPosition = tag.GetPosition();
                it->m_CVSS_score = tag.GetScore();
                it->m_nodeId = tag.GetNodeId();
            }
            break;
        }
    }

    if (!found)
    {
        NeighborInformation new_n;
        new_n.neighbor_mac = addr;
        new_n.last_beacon = Now ();
        if (useTag)
        {
            new_n.m_currentPosition = tag.GetPosition();
            new_n.m_CVSS_score = tag.GetScore();
            new_n.m_nodeId = tag.GetNodeId();
        }
        m_subordinates.push_back (new_n);
    }
}

void ASApplication::RemoveOldNeighbors ()
{
    auto it = m_neighbors.begin();
    while (it != m_neighbors.end())
    {
          //获取最后通信时间到现在的时间
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            it = m_neighbors.erase (it); // erase返回下一个迭代器
        }    
        else 
        {
            ++it;
        }
    }

    auto its = m_subordinates.begin();
    while (its != m_neighbors.end())
    {
          //获取最后通信时间到现在的时间
        Time last_contact = Now () - its->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            its = m_neighbors.erase (its); // erase返回下一个迭代器
        }    
        else 
        {
            ++its;
        }
    }

    //周期性检查并移除长时间未通信节点
    Simulator::Schedule (Seconds (1), &ASApplication::RemoveOldNeighbors, this);
}

void ASApplication::PrintNeighbors ()
{
    NS_LOG_INFO ( "NodeId:" << GetNode()->GetId() << " Role: "<< static_cast<int>(m_role) <<" Neighbor Size: " << m_neighbors.size() );
    // for (std::list<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    // {
    //     // std::cout << "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon << std::endl;
    //     NS_LOG_INFO ( "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon );
    // }
}

void ASApplication::SetTxInfo(TxInfo &p_tx)
{
    p_tx.channelNumber = CCH;
    p_tx.priority = 7;
    p_tx.txPowerLevel = 7;
    p_tx.dataRate = m_mode;
    p_tx.preamble = WIFI_PREAMBLE_LONG;
}

void ASApplication::SetASDataTag(ASDataTag & tag, uint32_t type)
{
    tag.SetMessageType(type);
    tag.SetNodeId ( GetNode()->GetId() );
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    tag.SetScore(CalcScore());
    tag.SetRole(m_role);
}

void ASApplication::SendMessage(uint32_t msg_type, const Address & addr)
{
    TxInfo tx;
    SetTxInfo(tx);

    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    ASDataTag tag;
    SetASDataTag(tag, msg_type);

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, addr, 0x88dc, tx);
}

void ASApplication::BroadcastInformation()
{
    //数据包参数
    TxInfo tx;
    SetTxInfo(tx);
    
    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    //tag中携带本节点的ID、位置信息、发送时间信息
    ASDataTag tag;
    SetASDataTag(tag, ASDataTag::BROADCAST);

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, Mac48Address::GetBroadcast(), 0x88dc, tx);

    //每m_broadcast_time广播一次
    Simulator::Schedule (m_broadcast_time, &ASApplication::BroadcastInformation, this);
}

void ASApplication::StartElection()
{   
    switch (m_role)
    {
    case HEAD:
    case SECONDARY:
        return;
        break;
    case CORE:
        ++m_vote_failure_count;
        break;
    default:
        break;
    }
    
    int n = m_neighbors.size(); // 邻居的数量就是RangeQuery的结果
    if (n < minPts)
    {   
        // m_role保持初始值Noise
        return;
    }

    m_role = CORE;
    
    SendMessage(ASDataTag::REQ_VOTE, Mac48Address::GetBroadcast());

    Simulator::Schedule (Seconds (2), &ASApplication::StartElection, this);
}

void ASApplication::SendHeartBeat()
{
    if(!(m_role==HEAD || m_role==SECONDARY))
    {
        return;
    }
    
    for (auto it = m_subordinates.begin(); it != m_subordinates.end(); ++it)
    {
        SendMessage(ASDataTag::HEARTBEAT, it->neighbor_mac);
    }

    Simulator::Schedule (Seconds (2), &ASApplication::SendHeartBeat, this);
}

bool ASApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    ASDataTag tag;
    bool useTag = packet->PeekPacketTag(tag);

    if (useTag)
    {   
        bool replyMessage = false;
        uint32_t message_type = 0;

        switch (tag.GetMessageType())
        {
        case ASDataTag::BROADCAST:
            NS_LOG_INFO ("\t消息发送方的ID: " << tag.GetNodeId() << "\t消息类型: " << tag.GetMessageType() << "\t消息发送方的位置 " << tag.GetPosition() 
                        << "\t消息发送时间: " << tag.GetTimestamp() << "\t消息延迟="<< Now()-tag.GetTimestamp() << "\tCVSS=" << tag.GetScore());
            break;
        case ASDataTag::REQ_VOTE: 
            // NS_LOG_INFO ( GetNode()->GetId() << "\t消息发送方的ID: " << tag.GetNodeId() << "\t消息类型: " << tag.GetMessageType() << "\t消息发送方的位置 " << tag.GetPosition() 
                        // << "\t消息发送时间: " << tag.GetTimestamp() << "\t消息延迟="<< Now()-tag.GetTimestamp() << "\tCVSS=" << tag.GetScore());
            switch (m_role)
            {
            case CORE:
            case SECONDARY:
                replyMessage = true;                
                message_type = CalcScore() > tag.GetScore() ? ASDataTag::DENY_VOTE : ASDataTag::APROVE_VOTE;
                break;
            case HEAD:
                replyMessage = true;
                if(CalcScore() > tag.GetScore())
                {
                    message_type = ASDataTag::APPOINTMENT;
                    ++m_vote_count;
                }
                else
                {
                    message_type = ASDataTag::APROVE_VOTE;
                    m_role = BORDER;
                    m_subordinates.clear();
                }
                break;
            default:
                m_role = BORDER;
                // NS_LOG_INFO ("Become Border");
                break;
            }
            break;
        case ASDataTag::DENY_VOTE:
            // nothing to do yet
            break;
        case ASDataTag::APROVE_VOTE:
            m_role = ++m_vote_count > (minVotes - m_vote_failure_count) ? HEAD : m_role;
            UpdateSubordinates (sender, tag, useTag);
            break;
        case ASDataTag::APPOINTMENT:
            m_role = SECONDARY;

            break;
        default:
            break;
        }
        
        if (replyMessage)
        {
            SendMessage(message_type, sender);
        }
    }     

    UpdateNeighbor (sender, tag, useTag);
    return true;
}
