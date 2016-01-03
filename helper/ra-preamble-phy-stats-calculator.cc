/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ra-preamble-phy-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <ns3/log.h>
#include <map>
#include <fstream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RaPreamblePhyStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED ( RaPreamblePhyStatsCalculator);

RaPreamblePhyStatsCalculator::RaPreamblePhyStatsCalculator ()
  : m_firstWriteImsiTime (true),
    m_pendingOutput (false) 
{
  NS_LOG_FUNCTION (this);
}

RaPreamblePhyStatsCalculator::~RaPreamblePhyStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
RaPreamblePhyStatsCalculator::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::RaPreamblePhyStatsCalculator")
    .SetParent<Object> ()
    .AddConstructor<RaPreamblePhyStatsCalculator> ()
    .SetGroupName("Lte")
    .AddAttribute ("StartTime", "Start time of the on going epoch.", 
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&RaPreamblePhyStatsCalculator::SetStartTime,
                                     &RaPreamblePhyStatsCalculator::GetStartTime), 
                   MakeTimeChecker ())
    .AddAttribute ("EpochDuration", "Epoch duration.", 
                   TimeValue (Seconds (0.25)), 
                   MakeTimeAccessor (&RaPreamblePhyStatsCalculator::GetEpoch,
                                     &RaPreamblePhyStatsCalculator::SetEpoch), 
                   MakeTimeChecker ())
    .AddAttribute ("PreambleRxOutputFilename",
                   "Name of the file where rach preamble Rx will be logged.",
                   StringValue ("PreamblePhyRx.txt"),
                   MakeStringAccessor (&RaPreamblePhyStatsCalculator::SetPreambleRxFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
RaPreamblePhyStatsCalculator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_pendingOutput)
    {
      ShowResults ();
    }
}

std::string
RaPreamblePhyStatsCalculator::GetPreambleRxFilename(void)
{
	return m_preambleRxFilename;
}

void
RaPreamblePhyStatsCalculator::SetPreambleRxFilename(std::string filename)
{
	m_preambleRxFilename = filename;
}

void 
RaPreamblePhyStatsCalculator::SetStartTime (Time t)
{
  m_startTime = t;
  RescheduleEndEpoch ();
}

Time 
RaPreamblePhyStatsCalculator::GetStartTime () const
{
  return m_startTime;
}

void 
RaPreamblePhyStatsCalculator::SetEpoch (Time e)
{
  m_epochDuration = e;
  RescheduleEndEpoch ();
}

Time 
RaPreamblePhyStatsCalculator::GetEpoch () const
{
  return m_epochDuration;  
}


void
RaPreamblePhyStatsCalculator::StorePreambleRx(PhyReceptionStatParameters params)
{
	NS_LOG_FUNCTION(this << "StorePreambleRx");
  Time now = Simulator::Now();
	RxPhyPreambleInfo_t rxPreambleInfo_t;
	rxPreambleInfo_t.m_cellId = params.m_cellId;
	rxPreambleInfo_t.m_imsi = params.m_imsi;
	rxPreambleInfo_t.m_correct = params.m_correctness;
  rxPreambleInfo_t.m_delay = NanoSeconds(now - params.m_timestamp);
	m_preambleRxEvents.insert(std::pair<Time, RxPhyPreambleInfo_t> (now, rxPreambleInfo_t)); 
	// store the informations in a container ordered by time	
	m_pendingOutput = true;
}

void
RaPreamblePhyStatsCalculator::RescheduleEndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  m_endEpochEvent.Cancel ();
  NS_ASSERT (Simulator::Now ().GetMilliSeconds () == 0); // below event time assumes this
  m_endEpochEvent = Simulator::Schedule (m_startTime + m_epochDuration, &RaPreamblePhyStatsCalculator::EndEpoch, this);
}

void
RaPreamblePhyStatsCalculator::EndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  ShowResults ();
  ResetResults ();
  m_startTime += m_epochDuration;
  m_endEpochEvent = Simulator::Schedule (m_epochDuration, &RaPreamblePhyStatsCalculator::EndEpoch, this);
}

void
RaPreamblePhyStatsCalculator::ResetResults (void)
{
  NS_LOG_FUNCTION (this);
  m_preambleRxEvents.erase (m_preambleRxEvents.begin (), m_preambleRxEvents.end ());
}

void
RaPreamblePhyStatsCalculator::ShowResults (void)
{	//info and dbug
	NS_LOG_INFO (this << GetPreambleRxFilename ().c_str ());
	NS_LOG_DEBUG ("Write Rach Preamble Stats in " << GetPreambleRxFilename ().c_str ());

	std::ofstream outFile;

	if (m_firstWriteImsiTime == true)
	{
	  outFile.open (GetPreambleRxFilename ().c_str ());
	  if (!outFile.is_open ())
	    {
	      NS_LOG_ERROR ("Can't open file " << GetPreambleRxFilename ().c_str ());
	      return;
	    }

	  m_firstWriteImsiTime = false;
	  outFile << "% time\tcellId\tIMSI\tcorrect\tdelay";
	  outFile << std::endl;
	}
	else
	{
	  outFile.open (GetPreambleRxFilename ().c_str (),  std::ios_base::app);
	  if (!outFile.is_open ())
	    {
	      NS_LOG_ERROR ("Can't open file " << GetPreambleRxFilename ().c_str ());
	      return;
	    }
	}

	WriteResults (outFile);
	m_pendingOutput = false;
}


void
RaPreamblePhyStatsCalculator::WriteResults (std::ofstream& outFile)
{
	NS_LOG_FUNCTION (this << "Write results");
	for (std::multimap<Time, RxPhyPreambleInfo_t>::iterator it = m_preambleRxEvents.begin (); it != m_preambleRxEvents.end (); ++it)
	{
	  outFile << it->first.GetNanoSeconds() / 1.0e9 << "\t";
	  outFile << it->second.m_cellId << "\t";
	  outFile << it->second.m_imsi << "\t";
	  outFile << (uint32_t)it->second.m_correct << "\t";
    outFile << it->second.m_delay.GetNanoSeconds() / 1.0e9;
	  outFile << std::endl;
	}

	outFile.close ();
}

void
RaPreamblePhyStatsCalculator::StorePreambleRx (Ptr<RaPreamblePhyStatsCalculator> raStats, std::string path,
                           PhyReceptionStatParameters params)
{
	NS_LOG_FUNCTION (raStats << path);
  	raStats->StorePreambleRx (params);
}

} // namespace ns3