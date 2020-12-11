#include "as-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ASDataTag");
NS_OBJECT_ENSURE_REGISTERED (ASDataTag);

ASDataTag::ASDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
}

ASDataTag::ASDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
}

ASDataTag::~ASDataTag() {
}

TypeId ASDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ASDataTag")
    .SetParent<Tag> ()
    .AddConstructor<ASDataTag> ();
  return tid;
}

TypeId ASDataTag::GetInstanceTypeId (void) const
{
  return ASDataTag::GetTypeId ();
}


uint32_t ASDataTag::GetSerializedSize (void) const
{
	//对应车辆的位置、发送数据包的时间、车辆的ID
	return sizeof(Vector) + sizeof (ns3::Time) + 3 * sizeof(uint32_t) + sizeof(double);
}

//注意Serialize中的顺序要和Deserialize中的一致
void ASDataTag::Serialize (TagBuffer i) const
{	
	//消息类型
	i.WriteU32 (m_message_type);

	//发送数据包的时间
	i.WriteDouble(m_timestamp.GetDouble());

	//车辆的位置
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);

	//车辆的ID
	i.WriteU32(m_nodeId);
	
	i.WriteU32(m_role);

	//车辆的安全评分
	i.WriteDouble(m_CVSS_score);
}


void ASDataTag::Deserialize (TagBuffer i)
{
	//消息类型
	m_message_type = i.ReadU32();

	//发送数据包的时间
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);;

	//车辆的位置
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble(); 
	m_currentPosition.z = i.ReadDouble();

	//车辆的ID
	m_nodeId = i.ReadU32();

	m_role = i.ReadU32();

	// 车辆的安全评分
	m_CVSS_score = i.ReadDouble();
}

void ASDataTag::Print (std::ostream &os) const
{
}

uint32_t ASDataTag::GetMessageType()
{
	return m_message_type;
} 

void ASDataTag::SetMessageType(uint32_t type)
{
	m_message_type = type;
}

uint32_t ASDataTag::GetNodeId() {
	return m_nodeId;
}

void ASDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}

Vector ASDataTag::GetPosition(void) {
	return m_currentPosition;
}

Time ASDataTag::GetTimestamp() {
	return m_timestamp;
}

void ASDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

void ASDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

double ASDataTag::GetScore() {
	return m_CVSS_score;
}

void ASDataTag::SetScore(double score) {
	m_CVSS_score = score;
}

uint32_t ASDataTag::GetRole()
{
	return m_role;
}

void ASDataTag::SetRole (uint32_t role)
{
	m_role = role;
}

} // namespace ns3 end