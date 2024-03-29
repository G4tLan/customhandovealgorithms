/*
 * This algorithm is developed based on the work by:
 * Rami Ahmad et al. from the paper:
 * Efficient Handover in LTE-A by Using Mobility 
 * Pattern and User Trajectory Prediction
 * 
 */

#ifndef SCRATCH_SONG_MOON_ALGORITHM_H
#define SCRATCH_SONG_MOON_ALGORITHM_H

#include <ns3/lte-handover-algorithm.h>
#include <ns3/lte-handover-management-sap.h>
#include <ns3/simple-ref-count.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/vector.h>
#include <ns3/Enbs.h>
#include <ns3/ptr.h>
#include <ns3/UE.h>
#include <vector>
#include <map>

namespace ns3
{
class songMoonAlgorithm : public LteHandoverAlgorithm
{
public:
    songMoonAlgorithm();
    virtual ~songMoonAlgorithm();

    static TypeId GetTypeId();

    struct historicalHandover {
        uint32_t sourceCellId;
        uint32_t targetCellId;
        double trajectoryAngle;
        Vector uePresentPosition;
    };

    //inherited from LTEhandoverAlgorithm
    virtual void SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s);
    virtual LteHandoverManagementSapProvider *GetLteHandoverManagementSapProvider();

    friend class MemberLteHandoverManagementSapProvider<songMoonAlgorithm>;
    struct RLFStats {
        int RLF;
        double HPI;
        double RLFLate;
        double RLFEarly;
    };
    static std::vector<uint16_t> cellsToUpdate;
    static std::vector<uint16_t> cellsPowerToUpdate;
    static int thresholdSet; //0 first, 1 second, 2 both

    static void updateParameters(RLFStats values, uint16_t _cellId) {
        int c = 0;
        if(values.RLFLate >= 0.03){
            c++;
        }
        if(values.RLFEarly >= 0.03){
            c++;
        }
        thresholdSet = c;
        if(_cellId){
            bool found = false;
            for(auto it: cellsToUpdate){
                if(it == _cellId){
                    found = true;
                    break;
                }
            }
            if(!found){
                cellsToUpdate.push_back(_cellId);
                if(values.HPI >= 0.3){
                    cellsPowerToUpdate.push_back(_cellId);
                }
            }
        }
    }
protected:
    //inherited from object
    virtual void DoInitialize();
    virtual void DoDispose();

    // inherited from LteHandoverAlgorithm as a Handover Management SAP implementation
    void DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults);

    void setCellId(uint16_t cellId);

private:
    uint8_t m_measId_A1;
    uint8_t m_measId_A2;
    uint8_t m_measId_A3;
    uint8_t m_measId_A4;
    uint8_t m_measId_A5;
    uint8_t m_servingCellThreshold;
    uint32_t cellId;
    LteHandoverManagementSapUser *m_handoverManagementSapUser;
    LteHandoverManagementSapProvider *m_handoverManagementSapProvider;
    //key value is ue rnti and cellid
    std::map<uint64_t, std::vector<songMoonAlgorithm::historicalHandover>> UeHistoricalHandover;
    std::map<uint64_t, uint16_t> handoverEvents;

    LteRrcSap::ReportConfigEutra reportConfigA1;
    LteRrcSap::ReportConfigEutra reportConfigA2;
    LteRrcSap::ReportConfigEutra reportConfigA3;
    LteRrcSap::ReportConfigEutra reportConfigA4;
    LteRrcSap::ReportConfigEutra reportConfigA5;

    int numOfEnbs;
    int thresholdChange;

    double transmitPower;

    bool hasBeenReset;

    void setupReportConfigurations();

    void printEvent(uint8_t event) ;
    uint32_t searchTargetEnb(Vector ueCurentPos, Vector uePreviousPosition);

    double calculateAngle(Vector p1, Vector p2) {
        return atan((p2.y - p1.y)/(p2.x - p1.x));
    }

    double calculateThetaC(Vector p1, Vector p2, Vector p3){
        //|p1 - p2| = c
        //|p2 - p3| = a
        //|p1 - p3| = b

        double a = ns3::CalculateDistance(p2,p3);
        double b = ns3::CalculateDistance(p1,p3);
        double c = ns3::CalculateDistance(p1,p2);

        double ratio = (b*b + c*c - a*a)/(2*b*c);
        // std::cout << "{ p1: " << p1 << ",p2: "<< p2 << ",p3: " << p3 << "} " << std::endl;
        // std::cout << "{ a: " << a << ",b: "<< b << ",c: " << c << "} ratio " << ratio << std::endl;
        return acos(std::max(std::min(ratio,1.0),-1.0));
    }

    void updateThreshold(uint16_t rnti);
    void updateTxPower();

};
} // namespace ns3

#endif