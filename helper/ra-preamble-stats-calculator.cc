/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ra-preamble-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <ns3/log.h>
#include <map>
#include <fstream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RaPreambleStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED ( RaPreambleStatsCalculator);

RaPreambleStatsCalculator::RaPreambleStatsCalculator ()
  : m_firstWriteImsiTime (true),
    m_pendingOutput (false) 
{
  NS_LOG_FUNCTION (this);
}

RaPreambleStatsCalculator::~RaPreambleStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
RaPreambleStatsCalculator::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::RaPreambleStatsCalculator")
    .SetParent<Object> ()
    .AddConstructor<RaPreambleStatsCalculator> ()
    .SetGroupName("Lte")
    .AddAttribute ("StartTime", "Start time of the on going epoch.", 
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&RaPreambleStatsCalculator::SetStartTime,
                                     &RaPreambleStatsCalculator::GetStartTime), 
                   MakeTimeChecker ())
    .AddAttribute ("EpochDuration", "Epoch duration.", 
                   TimeValue (Seconds (0.25)), 
                   MakeTimeAccessor (&RaPreambleStatsCalculator::GetEpoch,
                                     &RaPreambleStatsCalculator::SetEpoch), 
                   MakeTimeChecker ())
    .AddAttribute ("PreambleRxOutputFilename",
                   "Name of the file where rach preamble Rx will be logged.",
                   StringValue ("PreambleRxCollisions.txt"),
                   MakeStringAccessor (&RaPreambleStatsCalculator::SetPreambleRxFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
RaPreambleStatsCalculator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_pendingOutput)
    {
      ShowResults ();
    }
}

std::string
RaPreambleStatsCalculator::GetPreambleRxFilename(void)
{
	return m_preambleRxFilename;
}

void
RaPreambleStatsCalculator::SetPreambleRxFilename(std::string filename)
{
	m_preambleRxFilename = filename;
}

void 
RaPreambleStatsCalculator::SetStartTime (Time t)
{
  m_startTime = t;
  RescheduleEndEpoch ();
}

Time 
RaPreambleStatsCalculator::GetStartTime () const
{
  return m_startTime;
}

void 
RaPreambleStatsCalculator::SetEpoch (Time e)
{
  m_epochDuration = e;
  RescheduleEndEpoch ();
}

Time 
RaPreambleStatsCalculator::GetEpoch () const
{
  return m_epochDuration;  
}


void
RaPreambleStatsCalculator::StorePreamblePhyRx(PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION(this << "StorePreambleRx");
  Time now = Simulator::Now();
  Time startTime = NanoSeconds(params.m_timestamp);
  uint64_t imsi = params.m_imsi;
  TimeImsiPair_t timeImsiPair (now, imsi);

  RxPhyPreambleInfo_t rxPhyPreambleInfo_t;
  rxPhyPreambleInfo_t.m_cellId = params.m_cellId;
  rxPhyPreambleInfo_t.m_imsi = params.m_imsi;
  rxPhyPreambleInfo_t.m_correct = params.m_correctness;
  rxPhyPreambleInfo_t.m_delay = now - startTime;
  m_preamblePhyRxEvents.insert(std::pair<TimeImsiPair_t, RxPhyPreambleInfo_t> (timeImsiPair, rxPhyPreambleInfo_t)); 
  // store the informations in a container ordered by time  
  m_pendingOutput = true;
}



void
RaPreambleStatsCalculator::StorePreambleRx(Ptr<RachPreambleLteControlMessage> msg)
{
	NS_LOG_FUNCTION(this << "StorePreambleRx");
  Time now = Simulator::Now();
	Time startTime = msg->GetStartTime();
  double delay = (now-startTime).GetNanoSeconds() / 1.0e9;
  RxPreambleInfo_t rxPreambleInfo_t;
  rxPreambleInfo_t.m_imsi = msg->GetImsi();
  rxPreambleInfo_t.m_delay = delay;
  rxPreambleInfo_t.m_rapId = msg->GetRapId();
	m_preambleRxEvents.insert(std::pair<Time, RxPreambleInfo_t> (now, rxPreambleInfo_t)); 
	// store the informations in a container ordered by time of reception	
	m_pendingOutput = true;
}

void
RaPreambleStatsCalculator::RescheduleEndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  m_endEpochEvent.Cancel ();
  NS_ASSERT (Simulator::Now ().GetMilliSeconds () == 0); // below event time assumes this
  m_endEpochEvent = Simulator::Schedule (m_startTime + m_epochDuration, &RaPreambleStatsCalculator::EndEpoch, this);
}

void
RaPreambleStatsCalculator::EndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  ShowResults ();
  ResetResults ();
  m_startTime += m_epochDuration;
  m_endEpochEvent = Simulator::Schedule (m_epochDuration, &RaPreambleStatsCalculator::EndEpoch, this);
}

void
RaPreambleStatsCalculator::ResetResults (void)
{
  NS_LOG_FUNCTION (this);
  m_preambleRxEvents.erase (m_preambleRxEvents.begin (), m_preambleRxEvents.end ());
  m_preamblePhyRxEvents.erase (m_preamblePhyRxEvents.begin(), m_preamblePhyRxEvents.end());
}

void
RaPreambleStatsCalculator::ShowResults (void)
{	//info and dbug
	NS_LOG_INFO (this << GetPreambleRxFilename ().c_str ());
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
	  outFile << "% time\tcellId\tIMSI\trxok\trapId\tcoll\tdelay";
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
RaPreambleStatsCalculator::WriteResults (std::ofstream& outFile)
{
  NS_LOG_FUNCTION (this << "Write results");
  // this cycle will find all the preambles received but collided
  // use equal_range to get rach preamble recvd at same time
  EventList_t::iterator iterWhile = m_preambleRxEvents.begin();
  while(iterWhile != m_preambleRxEvents.end())
    {
      // get all the entries which are equal to the first
      std::pair< EventList_t::iterator, EventList_t::iterator> iterPair = m_preambleRxEvents.equal_range(iterWhile->first);
      std::map<uint8_t, uint32_t> rapIdMap;
      for (EventList_t::iterator iterLoop = iterPair.first; iterLoop != iterPair.second; ++iterLoop)
      {
        ++rapIdMap[iterLoop->second.m_rapId];

      }

      for (EventList_t::iterator iterLoop = iterPair.first; iterLoop != iterPair.second; ++iterLoop)
      {
        bool collision = rapIdMap.find(iterLoop->second.m_rapId)->second > 1;
        iterLoop->second.m_collision = collision;
        TimeImsiPair_t timeImsiPair (iterLoop->first, iterLoop->second.m_imsi);
        EventPhyList_t::iterator iterInner = m_preamblePhyRxEvents.find(timeImsiPair);
        if (iterInner != m_preamblePhyRxEvents.end())
          {
            iterInner->second.m_collision = collision;  
            iterInner->second.m_rapId = iterLoop->second.m_rapId;
          }
      }

      iterWhile = iterPair.second;
    }

  for(EventPhyList_t::iterator timeImsiPairIterator = m_preamblePhyRxEvents.begin(); timeImsiPairIterator != m_preamblePhyRxEvents.end();
      ++timeImsiPairIterator)
    {
      outFile.setf ( std::ios::fixed );
      outFile.precision(3);         
      outFile << timeImsiPairIterator->first.first.GetSeconds() << "\t";
      outFile << timeImsiPairIterator->second.m_cellId << "\t";
      outFile << timeImsiPairIterator->second.m_imsi << "\t";
      
      outFile << (uint32_t)timeImsiPairIterator->second.m_correct << "\t";

      if (timeImsiPairIterator->second.m_correct > 0)
        {
          outFile << timeImsiPairIterator->second.m_rapId << "\t";
          outFile << (uint32_t)timeImsiPairIterator->second.m_collision << "\t";
        } 
      else
        {
          outFile << "-\t-\t";
        }
      outFile.precision(6);
      outFile << timeImsiPairIterator->second.m_delay.GetNanoSeconds() / 1.0e9;
      outFile << std::endl;  
    }
	

	outFile.close ();
}

void
RaPreambleStatsCalculator::StorePreambleRx (Ptr<RaPreambleStatsCalculator> raStats, std::string path,
                           Ptr<RachPreambleLteControlMessage> msg)
{
	NS_LOG_FUNCTION (raStats << path);
  	raStats->StorePreambleRx (msg);
}

void
RaPreambleStatsCalculator::StorePreamblePhyRx (Ptr<RaPreambleStatsCalculator> raStats, std::string path,
                           PhyReceptionStatParameters params)
{
  NS_LOG_FUNCTION (raStats << path);
    raStats->StorePreamblePhyRx (params);
}

} // namespace ns3