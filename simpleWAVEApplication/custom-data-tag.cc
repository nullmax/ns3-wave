#include "custom-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("CustomDataTag");
NS_OBJECT_ENSURE_REGISTERED (CustomDataTag);

CustomDataTag::CustomDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
}

CustomDataTag::CustomDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
}

CustomDataTag::~CustomDataTag() {
}

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


uint32_t CustomDataTag::GetSerializedSize (void) const
{
	//对应车辆的位置、发送数据包的时间、车辆的ID
	return sizeof(Vector) + sizeof (ns3::Time) + sizeof(uint32_t);
}

//注意Serialize中的顺序要和Deserialize中的一致
void CustomDataTag::Serialize (TagBuffer i) const
{
	//发送数据包的时间
	i.WriteDouble(m_timestamp.GetDouble());

	//车辆的位置
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);

	//车辆的ID
	i.WriteU32(m_nodeId);
}


void CustomDataTag::Deserialize (TagBuffer i)
{
	//发送数据包的时间
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);;

	//车辆的位置
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();

	//车辆的ID
	m_nodeId = i.ReadU32();

}

void CustomDataTag::Print (std::ostream &os) const
{
}

uint32_t CustomDataTag::GetNodeId() {
	return m_nodeId;
}

void CustomDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}

Vector CustomDataTag::GetPosition(void) {
	return m_currentPosition;
}

Time CustomDataTag::GetTimestamp() {
	return m_timestamp;
}

void CustomDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

void CustomDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

}


