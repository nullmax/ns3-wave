#ifndef CUSTOM_DATA_TAG_H
#define CUSTOM_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

namespace ns3
{
class LowIdDataTag : public Tag {
public:

	//以下六个函数是Tag中的，必须重写
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//tag变量的set和get函数
	Vector GetPosition(void);
	uint32_t GetNodeId();
	Time GetTimestamp ();

	void SetPosition (Vector pos);
	void SetNodeId (uint32_t node_id);
	void SetTimestamp (Time t);

	uint32_t GetCID();
	void SetCID(uint32_t cid);
	
	uint32_t GetMessageType();
	void SetMessageType(uint32_t msg_type);

	double GetScore();
	void SetScore(double score);

	LowIdDataTag();
	LowIdDataTag(uint32_t node_id);

	virtual ~LowIdDataTag();
private:

	uint32_t m_messagetype;
	uint32_t m_nodeId; //节点的ID
	uint32_t m_clusterId; //节点的CID
	Vector m_currentPosition; //节点的位置
	Time m_timestamp; //数据包发送的时间
	double m_CVSS_score; // 节点安全评分
};
}

#endif 