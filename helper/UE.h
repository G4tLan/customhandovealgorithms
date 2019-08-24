/*
 * UE.h
 *
 *  Created on: 02 Jun 2019
 *      Author: gift
 */

#ifndef SCRATCH_UE_H
#define SCRATCH_UE_H

#include <ns3/lte-module.h>
#include <ns3/netanim-module.h>
#include <ns3/mobility-helper.h>

namespace ns3
{

class UE
{
public:
	struct historyPos
	{
		Vector p1;
		Vector p2;
	};
	//uses rnti and cellid in that order as key
	static std::map< uint64_t, UE::historyPos> uePositionHistory;
	UE(int, int, int, int, int,std::map<uint32_t,  ns3::Vector>);
	UE(int,int,int,int,std::map<uint32_t,  ns3::Vector>);//for testing
	void setNetAnimProperties(AnimationInterface *, int);
	NodeContainer *getUes()
	{
		return &UENodes;
	}

	void updateUePositionHistory();
	void createInitialUePositions(std::map<uint32_t,  ns3::Vector>);

private:
	int numOfUEs;
	NodeContainer UENodes;
	MobilityHelper UeMobilityHelper;
	MobilityHelper UeMobilityHelper60Kmh;
	MobilityHelper UeMobilityHelper120Kmh;
	Ptr<ListPositionAllocator> initPositionAlloc;
	int xCenter;
	int yCenter;
	int radius;
	double loggingDistance;
	int simulationTime;
	double calculateDistance(Vector p1, Vector p2)
	{
		double dist = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);

		return std::sqrt(dist);
	}
};
} // namespace ns3

#endif /* SCRATCH_UE_H_ */
