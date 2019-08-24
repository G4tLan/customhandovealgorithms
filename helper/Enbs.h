/*
 * Enbs.h
 *
 *  Created on: 01 Jun 2019
 *      Author: gift
 */

#ifndef SCRATCH_ENBS_H_
#define SCRATCH_ENBS_H_

#include <ns3/lte-module.h>
#include <ns3/netanim-module.h>
#include <ns3/position-allocator.h>
#include <ns3/mobility-helper.h>

namespace ns3 {
class Enbs {
public:
	enum Position_Types {
		HEX_MATRIX, STRAIGHT_LINE
	};

	struct X2ConnectionPair {
		int node1Index;
		int node2Index;
	};
	Enbs(int numOfEnbs, int distance, Enbs::Position_Types);
	NodeContainer* getEnbs() {
		return &enbNodes;
	}
	void ConnectClosestEnbX2Interface(Ptr<LteHelper>, Enbs::Position_Types);
	void setNetAnimProperties(AnimationInterface*, int);
	int GetNumOfEnbsInRow() {
		return numOfEnbsHorizontally;
	}
	int GetNumOfRows() {
		return numOfEnbRows;
	}
	static std::map<uint32_t, ns3::Vector> enbPositions;
	void populatePositions();
	static std::map<uint32_t, std::vector<uint32_t>> enbNeighbours;
	void populateNeighbours();
private:
	NodeContainer enbNodes;
	MobilityHelper enbMobility;
	int numOfEnb;
	int numOfEnbsHorizontally;
	int numOfEnbRows;
	std::vector<Enbs::X2ConnectionPair> pairs;

	Ptr<ListPositionAllocator> generateEnbLocationsHex(int numOfEnbs,
			int distance);
	Ptr<ListPositionAllocator> generateEnbLocationsStraight(int numOfEnbs,
			int distance);
	Enbs::X2ConnectionPair createPair(int a, int b){
		Enbs::X2ConnectionPair pair;
		pair.node1Index = a;
		pair.node2Index = b;
		return pair;
	}

	void ConnectClosestEnbX2InterfaceHex(Ptr<LteHelper>);
	void ConnectClosestEnbX2InterfaceStraight(Ptr<LteHelper>);
};

}

#endif /* SCRATCH_ENBS_H_ */
