#include "lowid-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("LowIdDataTag");
NS_OBJECT_ENSURE_REGISTERED (LowIdDataTag);

LowIdDataTag::LowIdDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
}

LowIdDataTag::LowIdDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
}

LowIdDataTag::~LowIdDataTag() {
}

TypeId LowIdDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LowIdDataTag")
    .SetParent<Tag> ()
    .AddConstructor<LowIdDataTag> ();
  return tid;
}

TypeId LowIdDataTag::GetInstanceTypeId (void) const
{
  return LowIdDataTag::GetTypeId ();
}


uint32_t LowIdDataTag::GetSerializedSize (void) const
{
	//对应车辆的位置、发送数据包的时间、车辆的ID
	return sizeof(Vector) + sizeof (ns3::Time) + 3 * sizeof(uint32_t);
}

//注意Serialize中的顺序要和Deserialize中的一致
void LowIdDataTag::Serialize (TagBuffer i) const
{
	//发送数据包的时间
	i.WriteDouble(m_timestamp.GetDouble());

	//车辆的位置
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);

	//车辆的ID
	i.WriteU32(m_nodeId);

	//车辆的CID
	i.WriteU32(m_clusterId);

	i.WriteU32(m_messagetype);
}


void LowIdDataTag::Deserialize (TagBuffer i)
{
	//发送数据包的时间
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);;

	//车辆的位置
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();

	//车辆的ID
	m_nodeId = i.ReadU32();

	//车辆的CID
	m_clusterId = i.ReadU32();

	m_messagetype = i.ReadU32();
}

void LowIdDataTag::Print (std::ostream &os) const
{
}

uint32_t LowIdDataTag::GetNodeId() {
	return m_nodeId;
}

void LowIdDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}

Vector LowIdDataTag::GetPosition(void) {
	return m_currentPosition;
}

Time LowIdDataTag::GetTimestamp() {
	return m_timestamp;
}

void LowIdDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

void LowIdDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

uint32_t LowIdDataTag::GetCID()
{
	return m_clusterId;
}

void LowIdDataTag::SetCID(uint32_t cid)
{
	m_clusterId = cid;
}

uint32_t LowIdDataTag::GetMessageType()
{
	return m_messagetype;
}
void LowIdDataTag::SetMessageType(uint32_t msg_type)
{
	m_messagetype = msg_type;
}

}


