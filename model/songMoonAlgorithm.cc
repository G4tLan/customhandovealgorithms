#include "songMoonAlgorithm.h"
#include <algorithm> 
#include <ns3/log.h>
#include <math.h>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("songMoonAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(songMoonAlgorithm);

songMoonAlgorithm::songMoonAlgorithm() : m_handoverManagementSapUser(0)
{
    m_handoverManagementSapProvider = new MemberLteHandoverManagementSapProvider<songMoonAlgorithm>(this);
    handoverEvents = {};
    hasBeenReset = true;
}

songMoonAlgorithm::~songMoonAlgorithm()
{
    std::cout << "destroying song Moon Algorithm" << std::endl;
}

TypeId songMoonAlgorithm::GetTypeId()
{
    static TypeId tid = TypeId("ns3::songMoonAlgorithm")
                            .SetParent<LteHandoverAlgorithm>()
                            .SetGroupName("Lte")
                            .AddConstructor<songMoonAlgorithm>()
                            .AddAttribute ("NumberOfNeighbours",
                                "number of neighbours",
                                UintegerValue (2),
                                MakeUintegerAccessor (&songMoonAlgorithm::numOfEnbs),
                                MakeUintegerChecker<uint8_t> ())
                            .AddAttribute ("ThresholdChange",
                                "number of neighbours",
                                UintegerValue (2),
                                MakeUintegerAccessor (&songMoonAlgorithm::thresholdChange),
                                MakeUintegerChecker<uint8_t> ())
                            .AddAttribute ("TxPower",
                                "number of neighbours",
                                DoubleValue (43),
                                MakeDoubleAccessor (&songMoonAlgorithm::transmitPower),
                                MakeDoubleChecker<double> ());

    return tid;
}

void songMoonAlgorithm::SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s)
{
    m_handoverManagementSapUser = s;
}

LteHandoverManagementSapProvider *songMoonAlgorithm::GetLteHandoverManagementSapProvider()
{
    return m_handoverManagementSapProvider;
}

void songMoonAlgorithm::setupReportConfigurations(){
    ns3::Time ttt = MilliSeconds(10);
    uint8_t hysteresisIeValue = EutranMeasurementMapping::ActualHysteresis2IeValue (7);
    //Event A1 setup
    reportConfigA1.eventId = LteRrcSap::ReportConfigEutra::EVENT_A1;
    reportConfigA1.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA1.threshold1.range = 20; //serving cell threshold
    reportConfigA1.timeToTrigger = 10; //ms
    reportConfigA1.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA1.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A1 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA1);

    //event A2
    reportConfigA2.eventId = LteRrcSap::ReportConfigEutra::EVENT_A2;
    reportConfigA2.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRP;
    reportConfigA2.threshold1.range = 20; //serving cell threshold
    reportConfigA2.timeToTrigger = ttt.GetMilliSeconds(); //ms
    reportConfigA2.hysteresis = hysteresisIeValue;
    reportConfigA2.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRP;
    reportConfigA2.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A2 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA2);

    //Event A3
    reportConfigA3.eventId = LteRrcSap::ReportConfigEutra::EVENT_A3;
    reportConfigA3.a3Offset = 0;
    reportConfigA3.hysteresis = hysteresisIeValue;
    reportConfigA3.timeToTrigger = ttt.GetMilliSeconds();
    reportConfigA3.reportOnLeave = false;
    reportConfigA3.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRP;
    reportConfigA3.reportInterval = LteRrcSap::ReportConfigEutra::MS1024;
    m_measId_A3 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover (reportConfigA3);

    //event A4
    reportConfigA4.eventId = LteRrcSap::ReportConfigEutra::EVENT_A4;
    reportConfigA4.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA4.threshold1.range = 0; // intentionally very low threshold
    reportConfigA4.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA4.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A4 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover (reportConfigA4);

    //event A5
    reportConfigA5.eventId = LteRrcSap::ReportConfigEutra::EVENT_A5;
    reportConfigA5.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA5.threshold1.range = 0; // intentionally very low threshold
    reportConfigA5.threshold2.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA5.threshold2.range = 0; // intentionally very low threshold
    reportConfigA5.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA5.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A5 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover (reportConfigA5);
}

void songMoonAlgorithm::DoInitialize()
{
    setupReportConfigurations();
    LteHandoverAlgorithm::DoInitialize();
}

void songMoonAlgorithm::DoDispose()
{
    delete m_handoverManagementSapProvider;
}

void songMoonAlgorithm::printEvent(uint8_t event)
{
    switch ((uint32_t)event)
    {
    case 1:
        std::cout << " event A2 occured ";
        break;
    case 2:
        std::cout << " event A4 occured ";
        break;
    default:
        std::cout << " unknown event occured ";
        break;
    }
}
std::vector<uint16_t> ns3::songMoonAlgorithm::cellsToUpdate(0);
std::vector<uint16_t> ns3::songMoonAlgorithm::cellsPowerToUpdate(0);
int ns3::songMoonAlgorithm::thresholdSet = -1;
void songMoonAlgorithm::setCellId(uint16_t _cellId) {
    cellId = (uint32_t)_cellId;
}



uint32_t songMoonAlgorithm::searchTargetEnb(Vector ueCurrentPosition, Vector uePreviousPosition) {
    std::vector<uint32_t> neighCellIds = Enbs::enbNeighbours.find(cellId)->second;
    struct weightData {
        uint32_t cellid;
        double dc;
        double Oc;
    };
    std::vector<weightData> candidateN ;

    double radius = 500;
    double angleThresh = M_PI/3;
   // double error = 0.01;
    // std::cout << ueCurrentPosition << " " << neighCellIds.size() << " neighs size t -> " << Simulator::Now().GetSeconds() << std::endl;
    for(auto i = neighCellIds.begin(); i != neighCellIds.end(); i++){
        Vector neighPos = Enbs::enbPositions.find(*i)->second;
        //std::cout << "[ neighPos ->" << neighPos << " uePrev->" << uePreviousPosition <<" ueCurrent->" << ueCurrentPosition << " ]" << std::endl;
        double dc = ns3::CalculateDistance(ueCurrentPosition, neighPos);
        double Oc = calculateThetaC(uePreviousPosition, ueCurrentPosition,
            Enbs::enbPositions.find(*i)->second);
        bool capacity = true;

        if(dc <= radius && Oc <= angleThresh && capacity ) {
            weightData d;
            d.cellid = *i;
            d.dc = dc;
            d.Oc = Oc;
            candidateN.push_back(d);
        }
    }
    weightData bestNeigh {0,1000, M_PI};
    double weight = 0;
    for(auto i = candidateN.begin(); i != candidateN.end(); i++){
        double w = 2 - (i->Oc/angleThresh + i->dc/radius);
        if(w > weight){
            bestNeigh = *i;
            weight = w;
        }
    }

    return bestNeigh.cellid;
}

void songMoonAlgorithm::DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults)
{

    //std::cout << measResults.rsrpResult << 
    if (measResults.measId == m_measId_A2)
    {
        Vector ueCurrentPosition = measResults.uePosition;
        Vector uePreviousPosition = ns3::UE::uePositionHistory.find(measResults.imsi)->second.p1;
        double beta = calculateAngle(uePreviousPosition, ueCurrentPosition);

        auto handoverIt = UeHistoricalHandover.find(measResults.imsi);
        bool handedOver = false;

        if(handoverIt != UeHistoricalHandover.end()){
            auto records = handoverIt->second;
            for(auto it = records.begin(); it != records.end(); it++){
                bool capacity = true;
                if(fabs(it->trajectoryAngle - beta) <= 0.0872665){
                    if( fabs(it->uePresentPosition.x - ueCurrentPosition.x ) <= 1.0 && 
                        fabs(it->uePresentPosition.y - ueCurrentPosition.y ) <= 1.0){
                            if(capacity){
                                if(it->targetCellId){
                                // std::cout << " handover exist " << it->targetCellId <<
                                //     " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                                    // std::cout << " rnti " << rnti << " target cell " << it->targetCellId << std::endl;
                                    m_handoverManagementSapUser->
                                        TriggerHandover(rnti,it->targetCellId);
                                    handedOver = true;
                                    break;
                                }
                            } else {
                                // std::cout << "search no capacity" << std::endl;
                                uint32_t targetCell = 
                                    searchTargetEnb(ueCurrentPosition,uePreviousPosition);
                                if(targetCell){
                                    it->targetCellId = targetCell;
                                    // std::cout << " handover update " << targetCell <<
                                    //     " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                                    // std::cout << " rnti " << rnti << " target cell " << targetCell << std::endl;
                                    m_handoverManagementSapUser
                                        ->TriggerHandover(rnti,targetCell);
                                    handedOver = true;
                                    break;
                                }
                            }
                    }
                }
            }
        }
        if (!handedOver)
        {
            //std::cout << "search no history" << std::endl;
            uint32_t targetCell = searchTargetEnb(ueCurrentPosition, uePreviousPosition);
            if (targetCell)
            {
                historicalHandover h = {cellId, targetCell, beta, ueCurrentPosition};
                if (handoverIt != UeHistoricalHandover.end())
                {
                    handoverIt->second.push_back(h);
                }
                else
                {
                    std::vector<songMoonAlgorithm::historicalHandover> vh;
                    vh.push_back(h);
                    UeHistoricalHandover.insert(std::make_pair(measResults.imsi, vh));
                }
                    auto ongoingIt = handoverEvents.find(measResults.imsi);
                    if(ongoingIt == handoverEvents.end()){
                        m_handoverManagementSapUser
                            ->TriggerHandover(rnti, targetCell);
                        // std::cout << " rnti " << rnti << " imsi " << measResults.imsi << " target cell " << targetCell<<
                        //     " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                        handoverEvents[measResults.imsi] = rnti;
                    } else {
                        if(ongoingIt->second != rnti){
                            ongoingIt->second = rnti;
                            m_handoverManagementSapUser
                            ->TriggerHandover(rnti, targetCell);
                            // std::cout << " rnti " << rnti << " imsi " << measResults.imsi << " target cell " << targetCell<<
                            //     " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                        }
                    }
            }
        }
    }
    updateThreshold(rnti);
    
}

void songMoonAlgorithm::updateThreshold(uint16_t rnti){
    bool found = false;
    int index = 0;
    for(auto it: cellsToUpdate){
        if(it == cellId){
            found = true;
            break;
        }
        index++;
    }
    if(found){
        if(thresholdSet == 0){
            reportConfigA1.threshold1.range += thresholdChange;
            reportConfigA2.threshold1.range += thresholdChange;
            reportConfigA5.threshold1.range += thresholdChange;
            reportConfigA4.threshold1.range -= thresholdChange;
            reportConfigA5.threshold2.range -= thresholdChange;
        } else if(thresholdSet == 1){
            reportConfigA1.threshold1.range -= thresholdChange;
            reportConfigA2.threshold1.range -= thresholdChange;
            reportConfigA5.threshold1.range -= thresholdChange;
            reportConfigA4.threshold1.range += thresholdChange;
            reportConfigA5.threshold2.range += thresholdChange;
        }
        uint8_t ulimit = 25;
        uint8_t llimit = 15; 
        if(reportConfigA1.threshold1.range >= ulimit){
            reportConfigA1.threshold1.range = ulimit;
        }
        if(reportConfigA2.threshold1.range >= ulimit){
            reportConfigA2.threshold1.range = ulimit;
        }
        if(reportConfigA5.threshold1.range >= ulimit){
            reportConfigA5.threshold1.range = ulimit;
        }
        if(reportConfigA4.threshold1.range <= llimit){
            reportConfigA4.threshold1.range = llimit;
        }
        if(reportConfigA4.threshold2.range <= llimit){
            reportConfigA4.threshold2.range = llimit;
        }
        if(reportConfigA1.threshold1.range <= llimit){
            reportConfigA1.threshold1.range = llimit;
        }
        if(reportConfigA2.threshold1.range <= llimit){
            reportConfigA2.threshold1.range = llimit;
        }
        if(reportConfigA5.threshold1.range <= llimit){
            reportConfigA5.threshold1.range = llimit;
        }
        if(reportConfigA4.threshold1.range >= ulimit){
            reportConfigA4.threshold1.range = ulimit;
        }
        if(reportConfigA5.threshold2.range >= ulimit){
            reportConfigA5.threshold2.range = ulimit;
        }
        m_handoverManagementSapUser->UpdateUEMeasReportConfig(reportConfigA1);
        m_handoverManagementSapUser->UpdateMeasConfigToUe(rnti,reportConfigA1);
        m_handoverManagementSapUser->UpdateUEMeasReportConfig(reportConfigA2);
        m_handoverManagementSapUser->UpdateMeasConfigToUe(rnti,reportConfigA2);
        m_handoverManagementSapUser->UpdateUEMeasReportConfig(reportConfigA3);
        m_handoverManagementSapUser->UpdateMeasConfigToUe(rnti,reportConfigA3);
        m_handoverManagementSapUser->UpdateUEMeasReportConfig(reportConfigA4);
        m_handoverManagementSapUser->UpdateMeasConfigToUe(rnti,reportConfigA4);
        m_handoverManagementSapUser->UpdateUEMeasReportConfig(reportConfigA5);
        m_handoverManagementSapUser->UpdateMeasConfigToUe(rnti,reportConfigA5);
        if((int)cellsToUpdate.size() > index){
            cellsToUpdate.erase(cellsToUpdate.begin()+index);
        }
    }
}

void songMoonAlgorithm::updateTxPower(){
    int index = 0;
    for(auto it: cellsPowerToUpdate){
        if(it == cellId){
            break;
        }
        index++;
    }
    transmitPower+=1;
    if(transmitPower >= 50){
        transmitPower = 50;
    }
    m_handoverManagementSapUser->UpdateTransmissionPower(transmitPower);
    if((int)cellsPowerToUpdate.size() > index){
        cellsPowerToUpdate.erase(cellsPowerToUpdate.begin()+index);
    }
}
} // namespace ns3
