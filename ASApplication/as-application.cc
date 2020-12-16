#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "as-application.h"

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
    m_broadcast_time = MilliSeconds (1000); 
    m_packetSize = 1000;
    m_time_limit = Seconds (5);
    m_mode = WifiMode("OfdmRate6MbpsBW10MHz");
    m_role = NOISE;
    m_vote_count = 0;
    m_vote_failure_count = 0;
    m_recalc_secore = true;
    m_current_superior = -1;
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
    m_app_id = n->GetId();
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
    Simulator::Schedule (Seconds (2), &ASApplication::RemoveOldNeighbors, this);

    NS_LOG_INFO("APP" << m_app_id << " started");
}

double ASApplication::CalcScore()
{   
    if (m_recalc_secore)
    {
        double scores[3];
        m_cvss.CalcScore(scores);
        m_score = scores[0]+scores[1]+scores[2] + rand() % 10;
        m_recalc_secore = false;
    }
    return m_score;
}

void ASApplication::SetBroadcastInterval (Time interval)
{
    m_broadcast_time = interval;
}

void ASApplication::SetWifiMode (WifiMode mode)
{
    m_mode = mode;
}

inline bool SameSign(double x, double y)
{
    return (x >= 0 && y >= 0) || (x < 0 && y < 0);
}

bool ASApplication::SameVelocityDirection(Vector v2)
{
    Vector v1 =  GetNode()->GetObject<MobilityModel>()->GetVelocity();
    return SameSign(v1.x, v2.x) && SameSign(v1.x, v2.x) && SameSign(v1.x, v2.x);
}

void ASApplication::UpdateNode(NeighborInformation & ni, ASDataTag & tag)
{
    ni.m_currentPosition = tag.GetPosition();
    ni.m_currentVelocity = tag.GetVelocity();
    ni.m_CVSS_score = tag.GetScore();
    ni.m_nodeId = tag.GetNodeId();
    ni.m_role = tag.GetRole();
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
                UpdateNode(*it, tag);
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
            UpdateNode(new_n, tag);
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
                UpdateNode(*it, tag);
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
            UpdateNode(new_n, tag);
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
            if((it->m_nodeId - m_current_superior) == 0)
            {
                m_current_superior = -1;
            }
            it = m_neighbors.erase (it); // erase返回下一个迭代器
        }    
        else 
        {
            ++it;
        }
    }

    if (m_current_superior < 0 && m_role != HEAD)
    {
        StartElection();
    }

    // 对下属发送心跳包,移除失效的下属
    it = m_subordinates.begin();
    while (it != m_subordinates.end())
    {
        //获取最后通信时间到现在的时间
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            it = m_subordinates.erase (it); // erase返回下一个迭代器
        }    
        else 
        {
            SendMessage(HEARTBEAT, it->neighbor_mac); // 发送心跳包
            ++it;
        }
    }

    if(m_subordinates.empty())
    {
        m_role = m_current_superior > 0 ? BORDER : NOISE;
        m_vote_count = 0;
    }
    //周期性检查并移除长时间未通信节点
    Simulator::Schedule (Seconds (1), &ASApplication::RemoveOldNeighbors, this);
}

void ASApplication::PrintNeighbors ()
{
    NS_LOG_INFO ( "NodeId:" << GetNode()->GetId() << " Role: "<< m_role << " Superior: " << m_current_superior <<" Neighbor Size: " << m_neighbors.size() << " Subordinates size: " << m_subordinates.size() );
    // for (std::list<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    // {
        // NS_LOG_INFO ( "\t邻居节点的ID: " << it->m_nodeId << " 邻居节点的角色:" << it->m_role << " 最后通信时间: " << it->last_beacon );
    // }
    for (std::list<NeighborInformation>::iterator it = m_subordinates.begin(); it != m_subordinates.end(); it++ )
    {
        NS_LOG_INFO ( "\t下属节点的ID: " << it->m_nodeId << " 下属节点的角色:" << it->m_role << " 最后通信时间: " << it->last_beacon );
    }
}

void ASApplication::SetTxInfo(TxInfo &tx)
{
    tx.channelNumber = CCH;
    tx.priority = 7;
    tx.txPowerLevel = 7;
    tx.dataRate = m_mode;
    tx.preamble = WIFI_PREAMBLE_LONG;
}

void ASApplication::SetASDataTag(ASDataTag & tag, const uint32_t & type)
{
    tag.SetMessageType(type);
    tag.SetNodeId ( GetNode()->GetId() );
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    tag.SetVelocity ( GetNode()->GetObject<MobilityModel>()->GetVelocity());
    tag.SetScore(CalcScore());
    tag.SetRole(m_role);
}

void ASApplication::SendMessage(const uint32_t & msg_type, const Address & addr)
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
    SendMessage(BROADCAST, Mac48Address::GetBroadcast());

    //每m_broadcast_time广播一次
    Simulator::Schedule (m_broadcast_time, &ASApplication::BroadcastInformation, this);
}

void ASApplication::StartElection()
{   
    switch (m_role)
    {
    case HEAD:
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
    m_vote_count = 0;

    SendMessage(REQ_VOTE, Mac48Address::GetBroadcast());
}

bool ASApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    ASDataTag tag;
    bool useTag = packet->PeekPacketTag(tag);

    if (useTag)
    {   
        if(!SameVelocityDirection(tag.GetVelocity()))
        {   
            // 方向不同，不是邻居
            return true;
        }
        NS_LOG_INFO ("ID: " << m_app_id << " Role: " << m_role << " Superior: " << m_current_superior << " 消息发送方的ID: " << tag.GetNodeId() << " 消息类型: " << tag.GetMessageType() << " 消息发送方的位置 " << tag.GetPosition() << " 消息发送时间: " << tag.GetTimestamp() << " 消息延迟="<< Now()-tag.GetTimestamp() << " CVSS=" << tag.GetScore());

        bool replyMessage = false;
        uint32_t message_type = 0;

        switch (tag.GetMessageType())
        {
        case BROADCAST:
            // nothing to do yet
            break;
        case REQ_VOTE: 
            replyMessage = true;                
            switch (m_role)
            {
            case CORE:
                message_type = CalcScore() > tag.GetScore() ? DENY_VOTE : APROVE_VOTE;
                break;
            case HEAD:
                if(CalcScore() > tag.GetScore())
                {
                    message_type = APPOINTMENT;
                }
                else
                {
                    message_type = APROVE_VOTE;
                    m_role = CORE;
                    m_vote_count = 0;
                }
                break;
            default:
                m_role = BORDER;
                m_vote_count = 0;
                if(m_current_superior < 0)
                {
                    message_type = APROVE_VOTE;
                    m_current_superior = tag.GetNodeId();
                }
                else
                {
                    message_type = DENY_VOTE;
                }
                // NS_LOG_INFO ("Noise Become Border");
                break;
            }
            break;
        case DENY_VOTE:
            // nothing to do yet
            break;
        case APROVE_VOTE:
            if(m_role >= CORE)
            {
                ++m_vote_count;
                if(m_role == CORE)
                {
                    m_role = m_vote_count > (minVotes - m_vote_failure_count) ? HEAD : m_role;
                }
                UpdateSubordinates (sender, tag, useTag);
            }
            break;
        case APPOINTMENT:
            if(m_role == CORE)
            {
                replyMessage = true;
                message_type = ACCEPT_APPOINTMENT;
            }
            break;
        case ACCEPT_APPOINTMENT:
            if(m_role == HEAD)
            {
                ++m_vote_count;
                UpdateSubordinates(sender, tag, useTag);
            }
            break;
        case HEARTBEAT:
            replyMessage = true;
            message_type = REPLY_HEARTBEAT;
            m_current_superior = tag.GetNodeId();
            break;
        case REPLY_HEARTBEAT:
            if (m_role >= CORE)
            {
                UpdateSubordinates(sender, tag, useTag);
            }
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
