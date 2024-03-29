#include "algorithmAdam.h"
#include <algorithm> 
#include <ns3/log.h>
#include <math.h>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("algorithmAdam");
NS_OBJECT_ENSURE_REGISTERED(algorithmAdam);

algorithmAdam::algorithmAdam() : m_handoverManagementSapUser(0)
{
    m_handoverManagementSapProvider = new MemberLteHandoverManagementSapProvider<algorithmAdam>(this);
    handoverEvents = {};
}

algorithmAdam::~algorithmAdam()
{
    std::cout << "destroying Adam algo" << std::endl;
}

TypeId algorithmAdam::GetTypeId()
{
    static TypeId tid = TypeId("ns3::algorithmAdam")
                            .SetParent<LteHandoverAlgorithm>()
                            .SetGroupName("Lte")
                            .AddConstructor<algorithmAdam>();

    return tid;
}

void algorithmAdam::SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s)
{
    m_handoverManagementSapUser = s;
}

LteHandoverManagementSapProvider *algorithmAdam::GetLteHandoverManagementSapProvider()
{
    return m_handoverManagementSapProvider;
}

void algorithmAdam::DoInitialize()
{
    ns3::Time ttt = MilliSeconds(10);
    uint8_t hysteresisIeValue = EutranMeasurementMapping::ActualHysteresis2IeValue (7);

    LteRrcSap::ReportConfigEutra reportConfigA2;
    reportConfigA2.eventId = LteRrcSap::ReportConfigEutra::EVENT_A2;
    reportConfigA2.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRP;
    reportConfigA2.threshold1.range = 20; //serving cell threshold
    reportConfigA2.timeToTrigger = ttt.GetMilliSeconds(); //ms
    reportConfigA2.hysteresis = hysteresisIeValue;
    reportConfigA2.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRP;
    reportConfigA2.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A2 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA2);

    LteRrcSap::ReportConfigEutra reportConfigA1;
    reportConfigA2.eventId = LteRrcSap::ReportConfigEutra::EVENT_A1;
    reportConfigA2.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA2.threshold1.range = 20; //serving cell threshold
    reportConfigA2.timeToTrigger = 10; //ms
    reportConfigA2.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA2.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A1 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA2);

    LteHandoverAlgorithm::DoInitialize();
}

void algorithmAdam::DoDispose()
{
    delete m_handoverManagementSapProvider;
}

void algorithmAdam::printEvent(uint8_t event)
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

void algorithmAdam::setCellId(uint16_t _cellId) {
    cellId = (uint32_t)_cellId;
}

uint32_t algorithmAdam::searchTargetEnb(Vector ueCurrentPosition, Vector uePreviousPosition) {
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

void algorithmAdam::DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults)
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
                    std::vector<algorithmAdam::historicalHandover> vh;
                    vh.push_back(h);
                    UeHistoricalHandover.insert(std::make_pair(measResults.imsi, vh));
                }
                    auto ongoingIt = handoverEvents.find(measResults.imsi);
                    if(ongoingIt == handoverEvents.end()){
                        m_handoverManagementSapUser
                            ->TriggerHandover(rnti, targetCell);
                        std::cout << " rnti " << rnti << " imsi " << measResults.imsi << " target cell " << targetCell<<
                            " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                        handoverEvents[measResults.imsi] = rnti;
                    } else {
                        if(ongoingIt->second != rnti){
                            ongoingIt->second = rnti;
                            m_handoverManagementSapUser
                            ->TriggerHandover(rnti, targetCell);
                            std::cout << " rnti " << rnti << " imsi " << measResults.imsi << " target cell " << targetCell<<
                                " t-> "<< Simulator::Now().GetSeconds() << std::endl;
                        }
                    }
            }
        }
    }
}
} // namespace ns3
