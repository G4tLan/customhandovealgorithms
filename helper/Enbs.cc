/*
 * Enbs.h
 *
 *  Created on: 01 Jun 2019
 *      Author: gift
 */

#include "Enbs.h"
#include <string>
#include <ctime>
#include <cstdlib>

namespace ns3 {

std::map<uint32_t, std::vector<uint32_t>> Enbs::enbNeighbours = {};

void Enbs::populateNeighbours()
{
	std::cout << "[" << std::endl;
	for (uint32_t i = 0; i < enbNodes.GetN(); i++)
	{
		Ptr<Node> n = enbNodes.Get(i);
		Ptr<ns3::LteEnbNetDevice> netD = n->GetDevice(0)->GetObject<LteEnbNetDevice>();

		if (netD != 0)
		{
			std::vector<uint32_t> neighbs;
			uint32_t cellId = (uint32_t)netD->GetCellId();

			for(auto it = pairs.begin(); it != pairs.end(); it++){
				if(it->node1Index == (int)i){
					neighbs.push_back(enbNodes.Get(it->node2Index)->GetDevice(0)
					->GetObject<ns3::LteEnbNetDevice>()
						->GetCellId());
				} else if(it->node2Index == (int)i) {
					neighbs.push_back(enbNodes.Get(it->node1Index)->GetDevice(0)
					->GetObject<ns3::LteEnbNetDevice>()
						->GetCellId());
				}
				if(neighbs.size() >= 6){
					break;
				}
			}

			enbNeighbours.insert(std::make_pair(cellId, neighbs));
			std::cout << "{ \n \t p: " << cellId << "\n \t n: [";
			for(auto ns = neighbs.begin(); ns != neighbs.end(); ns++){
				std::cout << *ns << " ";
			}
			std::cout << "] \n }," << std::endl;
		}
	}
	std::cout << "]" << std::endl;
}

std::map<uint32_t, ns3::Vector> Enbs::enbPositions = {};

void Enbs::populatePositions(){
	std::cout << "[" << std::endl;
	for(uint32_t i = 0; i < enbNodes.GetN(); i++){
		Ptr<ns3::LteEnbNetDevice> rrc = enbNodes.Get(i)->GetDevice(0)->GetObject<ns3::LteEnbNetDevice>();
		Ptr<ns3::MobilityModel> mob = enbNodes.Get(i)->GetObject<ns3::MobilityModel>();

		if(rrc != 0 && mob != 0){
			uint32_t cellId = (uint32_t) rrc->GetCellId();
			Vector position = mob->GetPosition();
			std::cout << "{ id: " << cellId << " , " << position << "}," << std::endl;

			enbPositions.insert(std::make_pair(cellId, position));
		}
	}
	std::cout << "]" << std::endl;
}

Enbs::Enbs(int numOfEnbs, int distance, Enbs::Position_Types p) {
	enbNodes.Create(numOfEnbs);
	numOfEnb = numOfEnbs;
	numOfEnbsHorizontally = 5;
	numOfEnbRows = 0;
	enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	switch (p) {
	case Enbs::Position_Types::HEX_MATRIX:
		enbMobility.SetPositionAllocator(
				generateEnbLocationsHex(numOfEnbs, distance));
		break;
	case Enbs::Position_Types::STRAIGHT_LINE:
	default:
		enbMobility.SetPositionAllocator(
				generateEnbLocationsStraight(numOfEnbs, distance));
	}
	enbMobility.Install(enbNodes);
}

Ptr<ListPositionAllocator> Enbs::generateEnbLocationsStraight(int numOfEnbs,
		int distance) {
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<
			ListPositionAllocator>();

	for (int i = 0; i < numOfEnbs; i++) {
		enbPositionAlloc->Add(Vector(i * distance, 50, 0));
	}

	return enbPositionAlloc;
}

bool checkExists(ListPositionAllocator L, Vector v, int& index) {
	double epsilon = 0.0001;
	Vector existing;
	for (uint32_t i = 0; i < L.GetSize(); i++) {
		Vector existing = L.GetNext();
		if (std::fabs(v.x - existing.x) <= epsilon) {
			if (std::fabs(v.y - existing.y) <= epsilon) {
				index = i;
				return true;
			}
		}
	}
	index = L.GetSize();
	return false;
}

std::vector<Vector> generatePoints(Vector refPoint, int distance) {
	std::vector<Vector> points;
	//0 rad
	points.push_back(Vector(refPoint.x + distance, refPoint.y, 0));
	// pi/3 rad
	points.push_back(
			Vector(refPoint.x + distance / 2,
					refPoint.y + distance * 0.86602540378, 0));
	// 2pi/3 rad
	points.push_back(
			Vector(refPoint.x - distance / 2,
					refPoint.y + distance * 0.86602540378, 0));
	// pi rad
	points.push_back(Vector(refPoint.x - distance, refPoint.y, 0));
	// 4pi/3 rad
	points.push_back(
			Vector(refPoint.x - distance / 2,
					refPoint.y - distance * 0.86602540378, 0));
	// 5pi/3 rad
	points.push_back(
			Vector(refPoint.x + distance / 2,
					refPoint.y - distance * 0.86602540378, 0));

	return points;
}

bool checkPairsExist(std::vector<Enbs::X2ConnectionPair> pairs, Enbs::X2ConnectionPair pair) {

	for(uint32_t  i = 0; i< pairs.size(); i++){
		Enbs::X2ConnectionPair temp = pairs.at(i);

		if((pair.node1Index == temp.node1Index && pair.node2Index == temp.node2Index)
				|| (pair.node1Index == temp.node2Index && pair.node2Index == temp.node1Index)){
			return true;
		}
	}
	return false;
}

Ptr<ListPositionAllocator> Enbs::generateEnbLocationsHex(int numOfEnbs,
		int distance) {
	{
		Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<
				ListPositionAllocator>();

		Vector refPoint = Vector(800, 800, 0);
		enbPositionAlloc->Add(refPoint);
		std::vector<Vector> points = generatePoints(refPoint, distance);

		numOfEnbs = numOfEnbs - 1;
		int i = 0, count = 0, refPointIndex = 0, pointIndex = -1, prevPointIndex = -1, startPointIndex = -1;
		 while (count < numOfEnbs) {
			if (!checkExists(*enbPositionAlloc, points.at(i), pointIndex)) {
				enbPositionAlloc->Add(points.at(i));
				/*
				 * parent refers refpoint
				 * child refers to point
				 */
				pairs.push_back(createPair(refPointIndex, pointIndex)); //pair parent with child
				//pair current child with previous child
				if(prevPointIndex > 0){ //exclude first child
					if(!checkPairsExist(pairs, createPair(prevPointIndex, pointIndex))){
						pairs.push_back(createPair(prevPointIndex, pointIndex));
					}
				}
				//last child i = 5
				count++;
			} else {
				if(!checkPairsExist(pairs, createPair(refPointIndex, pointIndex))){
					pairs.push_back(createPair(refPointIndex, pointIndex));
				}
			}
			prevPointIndex = pointIndex;
			if(i == 0) {
				startPointIndex = pointIndex;
			}
			if(i == 5){
				if(!checkPairsExist(pairs, createPair(startPointIndex, pointIndex))){
					pairs.push_back(createPair(startPointIndex, pointIndex));
				}
			}

			i = (i + 1) % 6;
			if (i == 0) {
				refPointIndex++;
				for(int ind = 0; ind <= refPointIndex; ind++){
					refPoint = enbPositionAlloc->GetNext();
				}
				for(int reset = 0; reset < count - refPointIndex; reset++){
					enbPositionAlloc->GetNext();
				}
				points = generatePoints(refPoint, distance);
			}
		};
		return enbPositionAlloc;
	}
}

void Enbs::ConnectClosestEnbX2InterfaceHex(Ptr<LteHelper> lteHelper) {
		int numberOfPairs = pairs.size();
		for(int pair = 0; pair < numberOfPairs; pair++){
			lteHelper->AddX2Interface(enbNodes.Get(pairs.at(pair).node1Index),
					enbNodes.Get(pairs.at(pair).node2Index));
		}
}

void Enbs::ConnectClosestEnbX2InterfaceStraight(Ptr<LteHelper> lteHelper) {
	for (int i = 1; i < numOfEnb; i++) {
		lteHelper->AddX2Interface(enbNodes.Get(i - 1), enbNodes.Get(i));
	}
}

void Enbs::ConnectClosestEnbX2Interface(Ptr<LteHelper> lteHelper,
		Enbs::Position_Types p) {
	switch (p) {
	case Enbs::Position_Types::HEX_MATRIX:
		ConnectClosestEnbX2InterfaceHex(lteHelper);
		break;
	case Enbs::Position_Types::STRAIGHT_LINE:
		ConnectClosestEnbX2InterfaceStraight(lteHelper);
		break;
	default:
		break;
	}
}

void Enbs::setNetAnimProperties(AnimationInterface* anim, int imageId) {
	for (int i = 0; i < numOfEnb; i++) {
		int nodeId = enbNodes.Get(i)->GetId();
		anim->UpdateNodeImage(nodeId, imageId);
		anim->UpdateNodeSize(nodeId, 25, 25);
		anim->UpdateNodeDescription(enbNodes.Get(i), 
		  "Enb");
	}
	anim = 0;
}

}
