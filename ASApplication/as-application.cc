#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "as-application.h"
#include "as-data-tag.h"

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
    m_broadcast_time = MilliSeconds (100); 
    m_packetSize = 1000;
    m_time_limit = Seconds (5);
    m_mode = WifiMode("OfdmRate6MbpsBW10MHz");
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
        Time random_offset = MicroSeconds (rand->GetValue(50,200));
        //m_broadcast_time秒钟后调用函数BroadcastInformation
        Simulator::Schedule (m_broadcast_time+random_offset, &ASApplication::BroadcastInformation, this);
    }
    else
    {
        NS_FATAL_ERROR ("There's no WaveNetDevice in your node");
    }
    //周期性检查邻居节点，并移除长时间未通信的节点
    Simulator::Schedule (Seconds (1), &ASApplication::RemoveOldNeighbors, this);
      
}

double ASApplication::CalcSecScore()
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

void ASApplication::BroadcastInformation()
{
    //数据包参数
    TxInfo tx;
    SetTxInfo(tx);
    
    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    //tag中携带本节点的ID、位置信息、发送时间信息
    ASDataTag tag;
    tag.SetNodeId ( GetNode()->GetId() );
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    tag.SetScore(CalcSecScore());

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, Mac48Address::GetBroadcast(), 0x88dc, tx);

    //每m_broadcast_time广播一次
    Simulator::Schedule (m_broadcast_time, &ASApplication::BroadcastInformation, this);
}

bool ASApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    ASDataTag tag;
    if (packet->PeekPacketTag (tag))
    {
        NS_LOG_INFO ("\t消息发送方的ID: " << tag.GetNodeId() << "\t消息发送方的位置 " << tag.GetPosition() 
                        << "\t消息发送时间: " << tag.GetTimestamp() << "\t消息延迟="<< Now()-tag.GetTimestamp() << "\tCVSS=" << tag.GetScore());
    }

    UpdateNeighbor (sender);

    return true;
}

void ASApplication::UpdateNeighbor (Address addr)
{
    bool found = false;
    
    for (std::list<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        if (it->neighbor_mac == addr)
        {
            it->last_beacon = Now();
            found = true;
            break;
        }
    }
    if (!found)
    {
        NeighborInformation new_n;
        new_n.neighbor_mac = addr;
        new_n.last_beacon = Now ();
        m_neighbors.push_back (new_n);
    }
}

void ASApplication::SetTxInfo(TxInfo &p_tx)
{
    p_tx.channelNumber = CCH;
    p_tx.priority = 7;
    p_tx.txPowerLevel = 7;
    p_tx.dataRate = m_mode;
    p_tx.preamble = WIFI_PREAMBLE_LONG;
}

void ASApplication::StartElection()
{   
    TxInfo tx;
    SetTxInfo(tx);

    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    //tag中携带本节点的ID、位置信息、发送时间信息
    ASDataTag tag;
    tag.SetNodeId ( GetNode()->GetId() );
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());
    tag.SetScore(CalcSecScore());

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, Mac48Address::GetBroadcast(), 0x88dc, tx);
}

void ASApplication::PrintNeighbors ()
{
    for (std::list<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        // std::cout << "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon << std::endl;
        NS_LOG_INFO ( "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon );
    }
    std::cout <<std::endl;
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
    
    //周期性检查并移除长时间未通信节点
    Simulator::Schedule (Seconds (1), &ASApplication::RemoveOldNeighbors, this);

}

