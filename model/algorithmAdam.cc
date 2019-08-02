#include "algorithmAdam.h"
#include <ns3/log.h>
#include "../../../scratch/UE.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("algorithmAdam");
NS_OBJECT_ENSURE_REGISTERED(algorithmAdam);

algorithmAdam::algorithmAdam() : m_handoverManagementSapUser(0)
{
    m_handoverManagementSapProvider = new MemberLteHandoverManagementSapProvider<algorithmAdam>(this);
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

    LteRrcSap::ReportConfigEutra reportConfigA2;
    reportConfigA2.eventId = LteRrcSap::ReportConfigEutra::EVENT_A2;
    reportConfigA2.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA2.threshold1.range = 30; //serving cell threshold
    reportConfigA2.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA2.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_measId_A2 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA2);

    LteRrcSap::ReportConfigEutra reportConfigA4;
    reportConfigA4.eventId = LteRrcSap::ReportConfigEutra::EVENT_A4;
    reportConfigA4.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA4.threshold1.range = 0; //serving cell threshold
    reportConfigA4.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA4.reportInterval = LteRrcSap::ReportConfigEutra::MS480;
    m_measId_A4 = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA4);

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

void algorithmAdam::DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults)
{

    if (measResults.measId == m_measId_A2)
    {
        //std::cout << Simulator::Now().GetSeconds() << " rnti " << rnti << ", sourcecell " << cellId << std::endl;
        
       //auto it = ns3::UE::uePositionHistory.find(std::make_pair(rnti, cellId));
       
       std::cout << measResults.uePosition.x << ", " << measResults.uePosition.y << std::endl;
    }
}
} // namespace ns3
