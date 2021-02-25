#ifndef CUSTOM_DATA_TAG_H
#define CUSTOM_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

namespace ns3
{

class ASDataTag : public Tag {
public:

	//以下六个函数是Tag中的，必须重写
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//tag变量的set和get函数
	uint32_t GetMessageType();
	Vector GetPosition(void);
	Vector GetVelocity(void);
	uint32_t GetNodeId();
	uint32_t GetRole();
	Time GetTimestamp ();
	double GetScore();

	void SetMessageType(uint32_t);
	void SetPosition (Vector pos);
	void SetVelocity (Vector velocity);
	void SetNodeId (uint32_t node_id);
	void SetRole (uint32_t role);
	void SetTimestamp (Time t);
	void SetScore(double score);


	ASDataTag();
	ASDataTag(uint32_t node_id);
	virtual ~ASDataTag();
private:
	uint32_t m_message_type;
	uint32_t m_nodeId; //节点的ID
	uint32_t m_role; //节点当前角色
	Vector m_currentPosition; //节点的位置
	Vector m_currentVelocity; // velocity;
	Time m_timestamp; //数据包发送的时间
	double m_CVSS_score; // 节点安全评分
};
}

#endif 