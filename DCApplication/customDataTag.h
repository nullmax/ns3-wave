#ifndef CUSTOM_DATA_TAG_H
#define CUSTOM_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

#include "urbanEnvironment.h"

using namespace ns3;

class CustomDataTag : public Tag {
public:

	
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//set函数
	// void SetTagType (uint32_t tt);
	// void SetNodeId (uint32_t node_id);
	// void SetvehicleType(uint32_t vehicleType);
	// void SetPosition (Vector pos);
	// void SetVelocity(Vector vel);//获取速度
	// void SetTimestamp (Time t);
	// void SetPacketSize (uint32_t ps);
	// void SetLeaderNodeDegree(double lnd);
    // void SetManagedNodeCounts(uint32_t mnc);
	// void SetAVPRole(uint32_t avpr);

	//get函数
	uint32_t GetTagType();
	uint32_t GetNodeId();
	uint32_t GetvehicleType();
	Vector GetPosition();
	uint32_t GetDirection();
	Vector GetVelocity();
	Time GetTimestamp ();
	uint32_t GetPacketSize();
	double GetLeaderNodeDegree();
    uint32_t GetManagedNodeCounts();
	uint32_t GetAVGRole();
	uint32_t GetHop2LeaderNode();
	

	//构造函数和析构函数
	CustomDataTag();
	CustomDataTag(uint32_t tagType,
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
												uint32_t hop);
	virtual ~CustomDataTag();
private:

	//tag的类型
	uint32_t m_tagType;
	//车辆节点的ID
	uint32_t m_nodeId;
	//车辆节点的类型
	uint32_t m_vehicleType;
	//车辆的位置
	Vector m_currentPosition;
	//车辆的运动方向
	uint32_t m_direction;
	//车辆的速度
	Vector m_currentVelocity;
	//tag的创建时间
	Time m_timestamp;
	//所发送的数据包的大小
	uint32_t packetSize;

	//车辆引领度
    double leaderNodeDegree;
    //节点管理节点的个数
    uint32_t managedNodeCounts; 
	//车辆在无人驾驶车群中的角色
    uint32_t AVGRole;
	//车辆到引领节点的跳数
	uint32_t Hop2LeaderNode;

};


#endif 