#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "custom-application.h"
#include "custom-data-tag.h"

NS_LOG_COMPONENT_DEFINE("CustomApplication");
NS_OBJECT_ENSURE_REGISTERED(CustomApplication);

TypeId CustomApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CustomApplication")
                .SetParent <Application> ()
                .AddConstructor<CustomApplication> ()
                .AddAttribute ("Interval", "Broadcast Interval",
                      TimeValue (MilliSeconds(100)),
                      MakeTimeAccessor (&CustomApplication::m_broadcast_time),
                      MakeTimeChecker()
                      )
                ;
    return tid;
}

TypeId CustomApplication::GetInstanceTypeId() const
{
    return CustomApplication::GetTypeId();
}

CustomApplication::CustomApplication()
{
    m_broadcast_time = MilliSeconds (100); 
    m_packetSize = 1000;
    m_time_limit = Seconds (5);
    m_mode = WifiMode("OfdmRate6MbpsBW10MHz");
}

CustomApplication::~CustomApplication()
{

}

void CustomApplication::StartApplication()
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
            dev->SetReceiveCallback (MakeCallback (&CustomApplication::ReceivePacket, this));

            break;
        } 
    }
    if (m_waveDevice)
    {
        //设置随机数
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        Time random_offset = MicroSeconds (rand->GetValue(50,200));
        //m_broadcast_time秒钟后调用函数BroadcastInformation
        Simulator::Schedule (m_broadcast_time+random_offset, &CustomApplication::BroadcastInformation, this);
    }
    else
    {
        NS_FATAL_ERROR ("There's no WaveNetDevice in your node");
    }
    //周期性检查邻居节点，并移除长时间未通信的节点
    Simulator::Schedule (Seconds (1), &CustomApplication::RemoveOldNeighbors, this);
      
}
void CustomApplication::SetBroadcastInterval (Time interval)
{
    m_broadcast_time = interval;
}

void CustomApplication::SetWifiMode (WifiMode mode)
{
    m_mode = mode;
}

void CustomApplication::BroadcastInformation()
{
    //数据包参数
    TxInfo tx;
    tx.channelNumber = CCH; 
    tx.priority = 7;
    tx.txPowerLevel = 7;
    tx.dataRate = m_mode;
    tx.preamble = WIFI_PREAMBLE_LONG;
    
    Ptr<Packet> packet = Create <Packet> (m_packetSize);
    
    //tag中携带本节点的ID、位置信息、发送时间信息
    CustomDataTag tag;
    tag.SetNodeId ( GetNode()->GetId() );
    tag.SetPosition ( GetNode()->GetObject<MobilityModel>()->GetPosition());

    //将tag加入数据包
    packet->AddPacketTag (tag);

    //将数据包以 WSMP (0x88dc)格式广播出去
    m_waveDevice->SendX (packet, Mac48Address::GetBroadcast(), 0x88dc, tx);

    //每m_broadcast_time广播一次
    Simulator::Schedule (m_broadcast_time, &CustomApplication::BroadcastInformation, this);
}

bool CustomApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    CustomDataTag tag;
    if (packet->PeekPacketTag (tag))
    {
        NS_LOG_INFO ("\t消息发送方的ID: " << tag.GetNodeId() << "\t消息发送方的位置 " << tag.GetPosition() 
                        << "\t消息发送时间: " << tag.GetTimestamp() << "\t消息延迟="<< Now()-tag.GetTimestamp());
    }

    UpdateNeighbor (sender);

    return true;
}

void CustomApplication::UpdateNeighbor (Address addr)
{
    bool found = false;
    
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
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

void CustomApplication::PrintNeighbors ()
{
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        std::cout << "\t邻居节点的地址: " << it->neighbor_mac << "\t最后通信时间: " << it->last_beacon << std::endl;
    }
    std::cout <<std::endl;
}

void CustomApplication::RemoveOldNeighbors ()
{
    for (std::vector<NeighborInformation>::iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++ )
    {
        //获取最后通信时间到现在的时间
        Time last_contact = Now () - it->last_beacon;

        if (last_contact >= m_time_limit) 
        {
            m_neighbors.erase (it);
            break;
        }    
    }
    //周期性检查并移除长时间未通信节点
    Simulator::Schedule (Seconds (1), &CustomApplication::RemoveOldNeighbors, this);

}

