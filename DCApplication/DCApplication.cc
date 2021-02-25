#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "customDataTag.h"
#include "DCApplication.h"

int DCApplication::head_count = 0;
int DCApplication::head_sum = 0;
int DCApplication::common_count = 0;
int DCApplication::common_sum = 0;
int DCApplication::m_msg_count = 0;

NS_LOG_COMPONENT_DEFINE("DCApplication");
NS_OBJECT_ENSURE_REGISTERED(DCApplication);

TypeId DCApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DCApplication")
                .SetParent <Application> ()
                .AddConstructor<DCApplication> ();
    return tid;
}

TypeId DCApplication::GetInstanceTypeId() const
{
    return DCApplication::GetTypeId();
}

//构造函数
DCApplication::DCApplication()
{
    m_recalc_secore = true;
}

//析构函数
DCApplication::~DCApplication()
{
    head_count += leadercounts;
    head_sum += leaderduration.GetSeconds();
    common_count += commoncounts;
    common_sum += commonduration.GetSeconds();

    cout<<leaderduration.GetSeconds()<<" "<<leadercounts<<" "<<commonduration.GetSeconds()<<" "<<commoncounts<<" "<<dataPacketsCounts<<endl;
}

//设置m_mode
void DCApplication::SetWifiMode (WifiMode mode)
{
    m_mode = mode;
}

//设置m_packetSize
void DCApplication::SetCommunicationPacketSize (uint32_t ps)
{
    m_packetSize = ps;
}

//设置m_broadcast_interval
void DCApplication::SetBroadcastInterval (Time interval)
{
    m_broadcast_interval = interval;
}

//设置m_neighbor_outofcontact_time_limit
void DCApplication::SetMyNeighborUutofcontactTimeLimit(Time interval)
{
    m_neighbor_outofcontact_time_limit = interval;
}

//设置m_remove_neighbor_frequency
void DCApplication::SetMyRemoveNeighborFrequency(Time frequency)
{
    m_remove_neighbor_frequency = frequency;
}

//设置m_autonomousVehicleGroupRoles
void DCApplication::SetAutonomousVehicleGroupRoles(int role)
{
    m_autonomousVehicleGroupRoles = role;
} 

//设置m_leaderNode_outofcontact_time_limit
void DCApplication::SetMyLeaderNodeOutofcontactTimeLimit(Time limit)
{
    m_leaderNode_outofcontact_time_limit = limit;
}

//设置m_managedNode_outofcontact_time_limit
void DCApplication::SetMyManagedNodeOutofcontactTimeLimit(Time limit)
{
    m_managedNode_outofcontact_time_limit = limit;
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

//应用程序开始后第一个调用的函数
void DCApplication::StartApplication()
{
    Ptr<Node> n = GetNode ();
    srand(n->GetId());
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

    for (uint32_t i = 0; i < n->GetNDevices (); i++)
    {
        Ptr<NetDevice> dev = n->GetDevice (i);
        if (dev->GetInstanceTypeId () == WaveNetDevice::GetTypeId())
        {
            m_waveDevice = DynamicCast <WaveNetDevice> (dev);
            dev->SetReceiveCallback (MakeCallback (&DCApplication::ReceivePacket, this));
            break;
        } 
    }
    if (m_waveDevice)
    {
        //更新节点的运动方向
        updateM_angle();

        //车辆通过Beacon获得周围节点的信息
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        Time random_offset = MicroSeconds (rand->GetValue(10,500));
        Simulator::Schedule (random_offset, &DCApplication::BroadcastInformationtoOneHopNeighbor, this);

        //开始选择core节点
        Ptr<UniformRandomVariable> r2 = CreateObject<UniformRandomVariable> ();
        Time ro2 = MicroSeconds (r2->GetValue(50,500));
        Simulator::Schedule (Seconds(1.1)+ro2, &DCApplication::SelectCoreNode, this);

        //周期性检查周围邻居节点信息、被管理节点信息
        Simulator::Schedule (Seconds(2), &DCApplication::IterateRemoveOldones, this);
        //测试输出
        Simulator::Schedule (Seconds(1), &DCApplication::PrintGroupInfo, this);
    }
    else
    {
        NS_FATAL_ERROR ("There's no WaveNetDevice in your node");
    }
}

double DCApplication::CalcScore()
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

//tag=HELLO：车辆节点广播数据包，用于获取周围邻居节点的信息
void DCApplication::BroadcastInformationtoOneHopNeighbor()
{
    SendMessageToaNode(BEACON, Mac48Address::GetBroadcast());
    
    Ptr<UniformRandomVariable> r3 = CreateObject<UniformRandomVariable> ();
    Time ro3 = MicroSeconds (r3->GetValue(100,500));
    Simulator::Schedule (m_broadcast_interval+ro3, &DCApplication::BroadcastInformationtoOneHopNeighbor, this);
}

//开始选择core节点
void DCApplication::SelectCoreNode()
{
    if (m_neighbors_autonomous.size()>MIN_DENSITY)
    {
        becomealeader(Now());
        m_autonomousVehicleGroupRoles = CORE;
        m_hopCountsToLeaderNode = 0;

        for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
        {
            if(it->neighbor_counts>MIN_DENSITY)
            {
                m_manangeAutonomousVehicles.push_back (*it);
                //如果邻居节点的密度满足条件，向邻居节点发送可以加入车群消息
                SendMessageToaNode(JOINRESP, Mac48Address::GetBroadcast());
            }
        }
    }
}

//周期性移除长时间没有通信的节点
void DCApplication::IterateRemoveOldones()
{
    //更新节点运动方向
    updateM_angle();

    //遍历无人邻居m_neighbors_autonomous，移除超过m_neighbor_outofcontact_time_limit未通信的节点
    for (std::list<NeighborAutonomousVehicleInfo>::iterator avit = m_neighbors_autonomous.begin(); avit != m_neighbors_autonomous.end();)
    {
        //过去最后一次通信到现在的时间间隔
        Time last_contact = Now () - avit->neighbor_last_beacon;
        //如果时间超过m_time_limit，将其移除
        if (last_contact >= m_neighbor_outofcontact_time_limit)
        {
            avit = m_neighbors_autonomous.erase (avit);
        }
        else
        {
            ++avit;
        }  
    }

    if(m_autonomousVehicleGroupRoles==CORE)
    {
        //本节点是CH，移除长时间未通信的被管理普通节点
        for (std::vector<NeighborAutonomousVehicleInfo>::iterator LNit = m_manangeAutonomousVehicles.begin(); LNit != m_manangeAutonomousVehicles.end();)
        {
            //过去最后一次通信到现在的时间间隔
            Time a_managedNode_last_contact_interval = Now () - LNit->neighbor_last_beacon;
            //如果时间超过m_managedNode_outofcontact_time_limit，将其移除
            if (a_managedNode_last_contact_interval > m_managedNode_outofcontact_time_limit)
            {
                LNit = m_manangeAutonomousVehicles.erase (LNit);
            }
            else
            {
                LNit++;
            }
        }

        //将密度满足条件的节点加入车群
        for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
        {
            if(it->AVPRole==FREE&&it->neighbor_counts>MIN_DENSITY)
            {
                m_manangeAutonomousVehicles.push_back (*it);
                SendMessageToaNode(JOINRESP, Mac48Address::GetBroadcast());
            }
        }

        //m_neighbors_autonomous.size()==0&&
        if(m_manangeAutonomousVehicles.size()==0&&
            (Now()-becomeleadertime)>Seconds(2))
        {   
            //记录放弃CORE节点的时间
            giveupleader(Now());
            m_autonomousVehicleGroupRoles = FREE;
            // cout<<"节点"<<GetNode()->GetId()<<"在时间"<<Now().GetSeconds()<<"因为在函数IterateRemoveOldones放弃CH节点"<<endl;
        }
    }
    else if(m_autonomousVehicleGroupRoles==ORDINARY&&m_hopCountsToLeaderNode==1)
    {
        //移除长时间未通信的被管理节点
        for (std::vector<NeighborAutonomousVehicleInfo>::iterator CNit = m_manangeAutonomousVehicles.begin(); CNit != m_manangeAutonomousVehicles.end();)
        {
            //过去最后一次通信到现在的时间间隔
            Time a_managedNode_last_contact_interval = Now () - CNit->neighbor_last_beacon;
            //如果时间超过m_managedNode_outofcontact_time_limit，将其移除
            if (a_managedNode_last_contact_interval > m_managedNode_outofcontact_time_limit)
            {
                CNit = m_manangeAutonomousVehicles.erase (CNit);
            } 
            else
            {
                 CNit++;
            }
        }

        //将密度满足条件的节点加入车群
        for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
        {
            if(it->AVPRole==FREE&&it->neighbor_counts>MIN_DENSITY)
            {
                m_manangeAutonomousVehicles.push_back (*it);
                SendMessageToaNode(JOINRESP, Mac48Address::GetBroadcast());
            }
        }
        
        //如果长时间未收到车群引领节点的信息，并且本节点满足CORE条件，则开始提升
        bool iwillUpgrade = false;
        Time my_leader_last_contact_interval = Now () - m_leaderNode.neighbor_last_beacon;
        if(my_leader_last_contact_interval > m_leaderNode_outofcontact_time_limit)
        {
            //CORE节点已断开连接
            iwillUpgrade = true;
        }

        //满足成为CORE的条件：CORE节点离开，并且被管理节点个数不为零
        if(iwillUpgrade)
        {
            if(m_manangeAutonomousVehicles.size()>0)
            {
                giveupcommon(Now());
                becomealeader(Now());
                // cout<<"节点"<<GetNode()->GetId()<<"在时间"<<Now().GetSeconds()<<"因为满足条件在函数IterateRemoveOldones晋升为CH节点"<<endl;

                m_autonomousVehicleGroupRoles = CORE; //本节点晋升为引领节点
                m_hopCountsToLeaderNode = 0; //本节点到引领节点的跳数是0

                for (std::vector<NeighborAutonomousVehicleInfo>::iterator LNit = m_manangeAutonomousVehicles.begin(); LNit != m_manangeAutonomousVehicles.end();LNit++)
                {
                    SendMessageToaNode(ORDINARYUP2CORE, LNit->neighbor_mac);
                }
                
            }
            else
            {
                giveupcommon(Now());
                // cout<<"节点"<<GetNode()->GetId()<<"在时间"<<Now().GetSeconds()<<"在函数IterateRemoveOldones放弃一跳CM节点"<<endl;

                m_autonomousVehicleGroupRoles = FREE;
            }
             
        }
        
    }
    else if(m_autonomousVehicleGroupRoles==ORDINARY&&m_hopCountsToLeaderNode==2)
    {
        //本节点是通过普通节点加入车群的普通节点，检查父节点是否长时间未通信
        //如果父节点长时间没有通信，则节点变为游离节点，重新广播，请求加入车群
        Time my_father_last_contact_interval = Now () - m_leaderNode.neighbor_last_beacon;
        if(my_father_last_contact_interval > m_leaderNode_outofcontact_time_limit)
        {
            //节点变为游离节点
            giveupcommon(Now());
            // cout<<"节点"<<GetNode()->GetId()<<"在时间"<<Now().GetSeconds()<<"在函数IterateRemoveOldones放弃二跳CM节点"<<endl;

            m_autonomousVehicleGroupRoles = FREE;
        }
    }
    else if(m_autonomousVehicleGroupRoles==FREE)
    {
        //如果FREE节点的邻居节点不存在CORE节点或距离CORE一跳的ORDINARY节点，并且密度满足要求，则转变为CORE节点

        bool laozizuiniubi = true;
        for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
        {
            if((it->AVPRole==CORE)||(it->AVPRole==ORDINARY&&it->hop2Leader==1))
            {
                laozizuiniubi = false;
                break;
            }
        }
    
        if (laozizuiniubi&&m_neighbors_autonomous.size()>MIN_DENSITY)
        {
            becomealeader(Now());
            m_autonomousVehicleGroupRoles = CORE;
            m_hopCountsToLeaderNode = 0;

            for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
            {
                if(it->AVPRole==FREE&&it->neighbor_counts>MIN_DENSITY)
                {
                    m_manangeAutonomousVehicles.push_back (*it);

                    //如果邻居节点的密度满足条件，向邻居节点发送可以加入车群消息
                    SendMessageToaNode(JOINRESP, Mac48Address::GetBroadcast());
                }
            }
        }
    }
    
    m_score_new = CalcScore();
    
    //更新引领节点和普通节点的生存时间
    if(iAmLeader)
    {
        leaderduration +=Seconds(1);   
        for (std::vector<NeighborAutonomousVehicleInfo>::iterator LNit = m_manangeAutonomousVehicles.begin(); LNit != m_manangeAutonomousVehicles.end();LNit++)
        {
            m_score_new += LNit->m_score;
        }
        m_score_new /= m_manangeAutonomousVehicles.size() + 1;
        cout << Now().GetSeconds() << " " << GetNode()->GetId() << " " << m_score_new << endl;
    }

    if(iAmCommon)
    {
        commonduration +=Seconds(1);   
    }

    if(m_autonomousVehicleGroupRoles==ORDINARY)
    {
        SendMessageToaNode(ORDINARY2CORE, m_leaderNode.neighbor_mac);
    }

    if(m_autonomousVehicleGroupRoles==CORE||(m_autonomousVehicleGroupRoles==ORDINARY&&m_hopCountsToLeaderNode == 1))
    {
        for(uint32_t bi=0; bi<m_manangeAutonomousVehicles.size(); ++bi)
        {
            SendMessageToaNode(CORE2ORDINARY, m_manangeAutonomousVehicles[bi].neighbor_mac);
        } 
    }

    //周期性遍历
    Simulator::Schedule (m_remove_neighbor_frequency, &DCApplication::IterateRemoveOldones, this);
}

//工具函数，向targetAddr发送messageType类型的消息
bool DCApplication::SendMessageToaNode(uint32_t messageType, Address targetAddr)
{
    ++m_msg_count;
    //发送数据包后，更新已发送数据包总量
    dataPacketsCounts += m_packetSize;
    
    TxInfo tx;
    tx.channelNumber = CCH; 
    tx.priority = 7;
    tx.txPowerLevel = 7;
    tx.dataRate = m_mode;
    tx.preamble = WIFI_PREAMBLE_LONG;
        
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
            
    //数据包的类型HeartBeatFromLeaderToCommon
    CustomDataTag tag = CustomDataTag(
        messageType,
            GetNode()->GetId(),
                m_neighbors_autonomous.size(),
                    GetNode()->GetObject<MobilityModel>()->GetPosition(),
                        m_angle,
                            GetNode()->GetObject<MobilityModel>()->GetVelocity(),
                                Simulator::Now(),
                                    m_packetSize,
                                        0,
                                            m_manangeAutonomousVehicles.size(),
                                                    m_autonomousVehicleGroupRoles,
                                                        m_hopCountsToLeaderNode);
                                                        
    tag.SetScore(m_score_new);

    packet->AddPacketTag (tag);
    return m_waveDevice->SendX (packet, targetAddr, 0x88dc, tx);
}

//接收到数据包的回调
bool DCApplication::ReceivePacket (Ptr<NetDevice> device, Ptr<const Packet> packet,uint16_t protocol, const Address &sender)
{
    //检查接收的数据包中是否携带tag数据
    CustomDataTag tag;
    //如果数据包中携带tag数据
    if (packet->PeekPacketTag (tag))
    {
        //所有与本车辆运动方向相同的节点发送的消息才会被才进行处理
        if(tag.GetDirection()!=m_angle)
            return true;

        uint32_t getTagType = tag.GetTagType();

        if(getTagType==BEACON)
        {
            //更新邻居无人驾驶车辆信息
            UpdateNeighborAutonomousVehicleWhenReceiveAPacket(
                tag.GetNodeId(),
                    sender,
                        tag.GetTimestamp(),
                            tag.GetPosition(),
                                tag.GetVelocity(),
                                    tag.GetAVGRole(),
                                        tag.GetHop2LeaderNode(),
                                            tag.GetManagedNodeCounts(),
                                                tag.GetvehicleType());
            
        }
        else if(getTagType==JOINRESP)
        {
            if(m_autonomousVehicleGroupRoles==FREE)
            {
                //收到同意加入请求
                receiveJOINRESP(sender, tag);
            }

        }
        else if(getTagType==CORE2ORDINARY)
        {
            //心跳包
            UpdateMyLeaderNodeStatus(
                sender,
                    tag.GetTimestamp(),
                        tag.GetPosition(),
                            tag.GetVelocity());
        }
        else if(getTagType==ORDINARY2CORE)
        {
            //心跳包
            UpdateMyManagedNodeStatus(
                sender,
                    tag.GetTimestamp(),
                        tag.GetPosition(),
                            tag.GetVelocity(),
                                tag.GetManagedNodeCounts(), tag.GetScore());
        }
        else if(getTagType==ORDINARYUP2CORE)
        {
            if(m_autonomousVehicleGroupRoles==ORDINARY&&m_hopCountsToLeaderNode==2)
            {
                m_autonomousVehicleGroupRoles = ORDINARY;
                m_hopCountsToLeaderNode = 1;
            }
        }
    }
    return true;
}

void DCApplication::UpdateNeighborAutonomousVehicleWhenReceiveAPacket(uint32_t nodeID, Address addr, 
    Time beaconTime, Vector currentPosition, Vector currentVelocity, 
        uint32_t role, uint32_t hop, uint32_t mnc, uint32_t nc)
{
    bool foundAVehicle = false;
    //遍历存储的邻居节点信息，如果发现已经有该节点，更新节点的信息
    for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
    {
        //当前邻居中存在该节点
        if (it->neighbor_mac == addr)
        {
            it->neighbor_last_beacon = beaconTime;
            it->neighbor_currentPosition = currentPosition;
            it->neighbor_currentVelocity = currentVelocity;
            it->AVPRole = role;
            it->hop2Leader = hop;
            it->managedNodeCounts = mnc;
            it->neighbor_counts = nc;
            foundAVehicle = true;
            break;
        }
    }
    if (!foundAVehicle) //如果没有找到该节点信息，创建一个节点，并将其存储
    {
        NeighborAutonomousVehicleInfo new_n;
        new_n.neighbor_ID = nodeID;
        new_n.neighbor_mac = addr;
        new_n.neighbor_first_bacon = beaconTime;
        new_n.neighbor_last_beacon = beaconTime;
        new_n.neighbor_currentPosition = currentPosition;
        new_n.neighbor_currentVelocity = currentVelocity;
        new_n.AVPRole = role;
        new_n.hop2Leader = hop;
        new_n.managedNodeCounts = mnc;
        new_n.neighbor_counts = nc;
        m_neighbors_autonomous.push_back (new_n);
    }
}

//节点发送请求后，收到回复
void DCApplication::receiveJOINRESP(const Address & addr, CustomDataTag & tag)
{
    // cout<<"节点"<<GetNode()->GetId()<<"在时间"<<Now().GetSeconds()<<"\t收到"<<nodeID<<"发送的允许加入车群消息"<<endl;

    //更新m_leaderNode
    m_leaderNode.neighbor_ID = tag.GetNodeId();
    m_leaderNode.neighbor_mac = addr;
    m_leaderNode.neighbor_first_bacon = tag.GetTimestamp();
    m_leaderNode.neighbor_last_beacon = tag.GetTimestamp();
    m_leaderNode.neighbor_currentPosition = tag.GetPosition();
    m_leaderNode.neighbor_currentVelocity = tag.GetVelocity();
    m_leaderNode.managedNodeCounts = tag.GetManagedNodeCounts();

    //设置m_autonomousVehicleGroupRoles
    m_autonomousVehicleGroupRoles = ORDINARY;
    becomeacommon(Now());

    //如果是m_hopCountsToLeaderNode=1，开始广播，允许车辆加入
    if(tag.GetAVGRole()==CORE)
    {
        //设置m_hopCountsToLeaderNode
        m_hopCountsToLeaderNode=1;

        for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
        {
            if(it->AVPRole==FREE&&it->neighbor_counts>MIN_DENSITY)
            {
                NeighborAutonomousVehicleInfo new_n;
                new_n.neighbor_ID = it->neighbor_ID;
                new_n.neighbor_mac = it->neighbor_mac;
                new_n.neighbor_first_bacon = Now();
                new_n.neighbor_last_beacon = Now();
                new_n.neighbor_currentPosition = it->neighbor_currentPosition;
                new_n.neighbor_currentVelocity = it->neighbor_currentVelocity;
                new_n.managedNodeCounts = it->managedNodeCounts;
                m_manangeAutonomousVehicles.push_back (new_n);
                //如果邻居节点的密度满足条件，向邻居节点发送可以加入车群消息，并将其加入
                SendMessageToaNode(JOINRESP, Mac48Address::GetBroadcast());
            }
        }
    }
    else
    {
        m_hopCountsToLeaderNode=2;
    }
}

//接收到CH发送的心跳包后，更新CH的信息
void DCApplication::UpdateMyLeaderNodeStatus(Address addr, Time beaconTime, Vector currentPosition, Vector currentVelocity)
{
    //确定发送心跳包的节点是否是存储的节点
    if(m_leaderNode.neighbor_mac == addr)
    {
        m_leaderNode.neighbor_last_beacon = beaconTime;
        m_leaderNode.neighbor_currentPosition = currentPosition;
        m_leaderNode.neighbor_currentVelocity = currentVelocity;
    }
}

//接收到被管理节点发送的心跳包后，更新被管理节点的信息
void DCApplication::UpdateMyManagedNodeStatus(Address addr, Time beaconTime, Vector currentPosition,
        Vector currentVelocity, uint32_t itsmanagedNodeCounts, double Score)
{
    for (std::vector<NeighborAutonomousVehicleInfo>::iterator it = m_manangeAutonomousVehicles.begin(); it != m_manangeAutonomousVehicles.end(); it++ )
    {
        if(it->neighbor_mac == addr)
        {
            it->neighbor_last_beacon = beaconTime;
            it->neighbor_currentPosition = currentPosition;
            it->neighbor_currentVelocity = currentVelocity;
            it->managedNodeCounts = itsmanagedNodeCounts;
            it->m_score = Score;
            break;
        }
    }
}

//计算车辆节点的运动方向
void DCApplication::updateM_angle()
{
    double m_sx = GetNode()->GetObject<MobilityModel>()->GetVelocity().x;
    double m_sy = GetNode()->GetObject<MobilityModel>()->GetVelocity().y;

    if(fabs(m_sx)<1&&fabs(m_sy)<1)
        return;

    if(fabs(m_sx)<1)
    {
        //南北方向移动
        if (m_sy > 0)
        {
            m_angle = 0;
        }
        else
        {
            m_angle = 180;
        }
    }
    else if (fabs(m_sy)<1)
    {
        //东西方向移动
        if (m_sx > 0)
        {
            m_angle = 90;
        }
        else
        {
            m_sx = 270;
        }
    }
}

//输出车群信息
void DCApplication::PrintGroupInfo()
{
    // string role;
    // Time roleDuration = Seconds(0);

    // if(m_autonomousVehicleGroupRoles==1)
    // {
    //     role = "核心节点状态";
    //     roleDuration = leaderduration;
    // }
    // else if(m_autonomousVehicleGroupRoles==2)
    // {
    //     role = "普通节点状态";
    //     roleDuration = commonduration;
    // }
    // else if(m_autonomousVehicleGroupRoles==3)
    //     role = "自由节点状态";
    
    // //输出不同时刻车群的信息
    // cout<<"TIME:"<<Now().GetSeconds()<<"\t节点："<<GetNode()->GetId()
    //     <<"\t"<<role
    //     <<"\t其管理的节点数目为："<<m_manangeAutonomousVehicles.size()
    //     <<"\t成为该角色时长："<<roleDuration.GetSeconds()<<"秒"
    //     <<"\t节点的位置为："<<GetNode()->GetObject<MobilityModel>()->GetPosition().x
    //     <<"\t"<<GetNode()->GetObject<MobilityModel>()->GetPosition().y
    //     <<"\t邻居节点数目："<<m_neighbors_autonomous.size()<<"\t"
    //     <<leaderduration.GetSeconds()<<"-"<<leadercounts<<"-"
    //     <<commonduration.GetSeconds()<<"-"<<commoncounts<<endl;


    // for (std::list<NeighborAutonomousVehicleInfo>::iterator it = m_neighbors_autonomous.begin(); it != m_neighbors_autonomous.end(); it++ )
    // {
    //     string nei_role;

    //     if(it->AVPRole==1)
    //         nei_role = "核心节点状态";
    //     else if(it->AVPRole==2)
    //         nei_role = "普通节点状态";
    //     else if(it->AVPRole==3)
    //         nei_role = "自由节点状态";

    //     cout<<"\t邻居节点："<<it->neighbor_ID
    //         <<"\t"<<nei_role
    //         <<"\t节点的位置为："<<it->neighbor_currentPosition.x
    //         <<"\t"<<it->neighbor_currentPosition.y
    //         <<"\t邻居个数:"<<it->neighbor_counts
    //         <<"\t加入时间"<<it->neighbor_first_bacon.GetSeconds()
    //         <<"\t最后更新时间"<<it->neighbor_last_beacon.GetSeconds()<<endl;
    // }

    if(m_autonomousVehicleGroupRoles==CORE)
    {
        int nei_manage_counts = m_manangeAutonomousVehicles.size();

    for (std::vector<NeighborAutonomousVehicleInfo>::iterator it = m_manangeAutonomousVehicles.begin(); it != m_manangeAutonomousVehicles.end(); it++ )
    {
        nei_manage_counts +=it->managedNodeCounts;
    }
        // cout<<Now().GetSeconds()<<" "<<nei_manage_counts<<endl;
    }

    Simulator::Schedule (Seconds(1.0), &DCApplication::PrintGroupInfo, this);

}

void DCApplication::becomealeader(Time t)
{
    iAmLeader = true;
    becomeleadertime = t;
    leadercounts++;
}

void DCApplication::giveupleader(Time t)
{
    iAmLeader = false;
}

void DCApplication::becomeacommon(Time t)
{
    iAmCommon = true;
    becomecommontime = t;
    commoncounts++;
}

void DCApplication::giveupcommon(Time t)
{
    iAmCommon = false;
}