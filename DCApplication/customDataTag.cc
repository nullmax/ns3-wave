#include "customDataTag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CustomDataTag");
NS_OBJECT_ENSURE_REGISTERED (CustomDataTag);

//构造函数
CustomDataTag::CustomDataTag() {
	m_tagType = 0;
	m_nodeId = 0;
	m_vehicleType = 0;
	m_currentPosition = Vector(0,0,0);
	m_direction = 0; 
	m_currentVelocity = Vector(0,0,0);
	m_timestamp = Simulator::Now();
	packetSize =0;
	leaderNodeDegree = 0;
	managedNodeCounts = 0;
	AVGRole = 0;
	Hop2LeaderNode = 0;
}

/**
 * 带参数的构造函数
 * tagType：tag的类型
 * node_id：节点的ID
 * vehicleType：车辆的类型
 * Position：车辆的位置
 * direction：车辆的运动方向 
 * Velocity：车辆的速度
 * timestamp：发送数据包的时间
 * ps：数据包的大小
 * lnd：车辆的引领节点度
 * mnc：车辆管理节点的个数
 * role：无人驾驶车辆在车群中的角色
 * hop：无人驾驶车辆到引领节点的跳数
*/
CustomDataTag::CustomDataTag(uint32_t tagType,
	uint32_t node_id,
		uint32_t vehicleType,
			Vector Position,
				uint32_t direction,
					Vector Velocity,
						Time timestamp, 
							uint32_t ps,
								double lnd,
									uint32_t mnc,
										uint32_t role,
											uint32_t hop) {
	m_tagType = tagType;
	m_nodeId = node_id;
	m_vehicleType = vehicleType;
	m_currentPosition = Position;
	m_direction = direction;
	m_currentVelocity = Velocity;
	m_timestamp = timestamp;
	packetSize = ps;
	leaderNodeDegree = lnd;
	managedNodeCounts = mnc;
	AVGRole = role;
	Hop2LeaderNode = hop;
}

//析构函数
CustomDataTag::~CustomDataTag() 
{
}

//Almost all custom tags will have similar implementation of GetTypeId and GetInstanceTypeId
TypeId CustomDataTag::GetTypeId (void)
{
  	static TypeId tid = TypeId ("ns3::CustomDataTag")
    	.SetParent<Tag> ()
    	.AddConstructor<CustomDataTag> ();
  	return tid;
}
TypeId CustomDataTag::GetInstanceTypeId (void) const
{
	return CustomDataTag::GetTypeId ();
}

// The size required for the data contained within tag 
uint32_t CustomDataTag::GetSerializedSize (void) const
{
	//tag的类型+节点ID+节点类型+位置+速度+时间+数据包大小+引领节点度+被管理节点个数+车群在无人驾驶车群中的角色+距离引领节点的跳数
	return sizeof(uint32_t) 
		+ sizeof(uint32_t) 
			+ sizeof(uint32_t) 
				+ sizeof(Vector) 
					+ sizeof(uint32_t)
						+ sizeof(Vector) 
							+ sizeof (ns3::Time) 
								+ sizeof(uint32_t)
									+ sizeof(double)
										+ sizeof(uint32_t)
											+ sizeof(uint32_t)
												+ sizeof(uint32_t)
													+sizeof(double); 
}

/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void CustomDataTag::Serialize (TagBuffer i) const
{
	//tag type
	i.WriteU32(m_tagType);
	//node ID
	i.WriteU32(m_nodeId);
	//车辆节点类型
	i.WriteU32(m_vehicleType);
	//position
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);
	//directionm
	i.WriteU32(m_direction);
	//velocity
	i.WriteDouble (m_currentVelocity.x);
	i.WriteDouble (m_currentVelocity.y);
	i.WriteDouble (m_currentVelocity.z);
	//timestamp
	i.WriteDouble(m_timestamp.GetDouble());
	//数据包大小
	i.WriteU32(packetSize);
	//车辆引领度
	i.WriteDouble(leaderNodeDegree);
	//节点管理节点的个数 
	i.WriteU32(managedNodeCounts);
	//无人驾驶车辆在车群中的角色
	i.WriteU32(AVGRole);
	//无人驾驶车辆到引领节点的跳数
	i.WriteU32(Hop2LeaderNode);

	i.WriteDouble(m_score);

}
/** This function reads data from a buffer and store it in class's instance variables.
 */
void CustomDataTag::Deserialize (TagBuffer i)
{
	//tag type
	m_tagType = i.ReadU32();
	//node id
	m_nodeId = i.ReadU32();
	//车辆节点类型
	m_vehicleType = i.ReadU32();
	//position
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();
	//direction
	m_direction = i.ReadU32();
	//velocity
	m_currentVelocity.x = i.ReadDouble();
	m_currentVelocity.y = i.ReadDouble();
	m_currentVelocity.z = i.ReadDouble();
	//timestamp
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);
	//数据包大小
	packetSize = i.ReadU32();
	//车辆引领度
	leaderNodeDegree = i.ReadDouble();
	//节点管理节点的个数 
	managedNodeCounts = i.ReadU32();
	//无人驾驶车辆在车群中的角色
	AVGRole = i.ReadU32();
	//无人驾驶车辆到引领节点的跳数
	Hop2LeaderNode = i.ReadU32();
	m_score = i.ReadDouble();
}
/**
 * This function can be used with ASCII traces if enabled. 
 */
void CustomDataTag::Print (std::ostream &os) const
{
  os << "Custom Data --- Node :" << m_nodeId <<  "\t(" << m_timestamp  << ")" << " Pos (" << m_currentPosition << ")"<<"vel("<<m_currentVelocity<<")";
}

//Your set functions 
// void CustomDataTag::SetTagType(uint32_t tt) {
// 	m_tagType = tt;
// }

// void CustomDataTag::SetNodeId(uint32_t node_id) {
// 	m_nodeId = node_id;
// }

// void CustomDataTag::SetvehicleType(uint32_t vehicleType) {
// 	m_vehicleType = vehicleType;
// }

// void CustomDataTag::SetPosition(Vector pos) {
// 	m_currentPosition = pos;
// }

// void CustomDataTag::SetVelocity(Vector vel) {
// 	m_currentVelocity = vel;
// }

// void CustomDataTag::SetTimestamp(Time t) {
// 	m_timestamp = t;
// }

// void CustomDataTag::SetPacketSize (uint32_t ps)
// {
// 	packetSize = ps;
// }

// void CustomDataTag::SetLeaderNodeDegree(double lnd)
// {
//     leaderNodeDegree = lnd;
// }

// void CustomDataTag::SetManagedNodeCounts(uint32_t mnc)
// {
//     managedNodeCounts = mnc;
// }

//Your get functions
uint32_t CustomDataTag::GetTagType() {
	return m_tagType;
}

uint32_t CustomDataTag::GetNodeId() {
	return m_nodeId;
}

uint32_t CustomDataTag::GetvehicleType()
{
	return m_vehicleType;
}

Vector CustomDataTag::GetPosition(void) {
	return m_currentPosition;
}

uint32_t CustomDataTag::GetDirection()
{
	return m_direction;
}

Vector CustomDataTag::GetVelocity(void) {
	return m_currentVelocity;
}

Time CustomDataTag::GetTimestamp() {
	return m_timestamp;
}

uint32_t CustomDataTag::GetPacketSize()
{
	return packetSize;
}

double CustomDataTag::GetLeaderNodeDegree()
{
    return leaderNodeDegree;
}

uint32_t CustomDataTag::GetManagedNodeCounts()
{
    return managedNodeCounts;
}

uint32_t CustomDataTag::GetAVGRole()
{
	return AVGRole;
}

uint32_t CustomDataTag::GetHop2LeaderNode()
{
	return Hop2LeaderNode;
}

double CustomDataTag::GetScore()
{
	return m_score;
}

void CustomDataTag::SetScore(double Score)
{
	m_score = Score;
}