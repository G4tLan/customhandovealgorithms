#include "algorithmAdam.h"
#include <ns3/log.h>

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

void algorithmAdam::SetLteHandoverManagementSapUser(LteHandoverManagementSapUser* s)
{
    m_handoverManagementSapUser = s;
}

LteHandoverManagementSapProvider* algorithmAdam::GetLteHandoverManagementSapProvider()
{
    return m_handoverManagementSapProvider;
}

void algorithmAdam::DoInitialize()
{
    LteRrcSap::ReportConfigEutra reportConfigA2;
    reportConfigA2.eventId = LteRrcSap::ReportConfigEutra::EVENT_A2;
    reportConfigA2.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA2.threshold1.range = 2;
    reportConfigA2.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA2.reportInterval = LteRrcSap::ReportConfigEutra::MS240;
    m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA2);

    LteRrcSap::ReportConfigEutra reportConfig;
    reportConfig.eventId = LteRrcSap::ReportConfigEutra::EVENT_A3;
    reportConfig.a3Offset = 0;
    reportConfig.hysteresis = 1;
    reportConfig.timeToTrigger = 500;
    reportConfig.reportOnLeave = false;
    reportConfig.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRP;
    reportConfig.reportInterval = LteRrcSap::ReportConfigEutra::MS1024;
    m_measId = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfig);

    LteRrcSap::ReportConfigEutra reportConfigA4;
    reportConfigA4.eventId = LteRrcSap::ReportConfigEutra::EVENT_A4;
    reportConfigA4.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
    reportConfigA4.threshold1.range = 0; // intentionally very low threshold
    reportConfigA4.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRQ;
    reportConfigA4.reportInterval = LteRrcSap::ReportConfigEutra::MS480;
    m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfigA4);

    LteHandoverAlgorithm::DoInitialize();
}

void algorithmAdam::DoDispose() 
{
    delete m_handoverManagementSapProvider;
}

void algorithmAdam::DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults)
{
    std::cout << "reporting by: " << rnti << " " << measResults.rsrpResult << std::endl;
}

} // namespace ns3
