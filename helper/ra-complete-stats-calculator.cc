/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ra-complete-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <ns3/log.h>
#include <map>
#include <fstream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RaCompleteStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED ( RaCompleteStatsCalculator);

RaCompleteStatsCalculator::RaCompleteStatsCalculator ()
  : m_firstWriteImsiTime (true),
    m_pendingOutput (false) 
{
  NS_LOG_FUNCTION (this);
}

RaCompleteStatsCalculator::~RaCompleteStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
RaCompleteStatsCalculator::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::RaCompleteStatsCalculator")
    .SetParent<Object> ()
    .AddConstructor<RaCompleteStatsCalculator> ()
    .SetGroupName("Lte")
    .AddAttribute ("StartTime", "Start time of the on going epoch.", 
                   TimeValue (Seconds (0.)),
                   MakeTimeAccessor (&RaCompleteStatsCalculator::SetStartTime,
                                     &RaCompleteStatsCalculator::GetStartTime), 
                   MakeTimeChecker ())
    .AddAttribute ("EpochDuration", "Epoch duration.", 
                   TimeValue (Seconds (0.25)), 
                   MakeTimeAccessor (&RaCompleteStatsCalculator::GetEpoch,
                                     &RaCompleteStatsCalculator::SetEpoch), 
                   MakeTimeChecker ())
    .AddAttribute ("RachDelayFilename",
                   "Name of the file where the time to complete RACH procedure will be logged.",
                   StringValue ("RaCompleted.txt"),
                   MakeStringAccessor (&RaCompleteStatsCalculator::SetRachDelayFilename),
                   MakeStringChecker ())
  ;
  return tid;
}

void
RaCompleteStatsCalculator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_pendingOutput)
    {
      ShowResults ();
    }
}


std::string
RaCompleteStatsCalculator::GetRachDelayFilename(void)
{
  return m_raCompletedFilename;
}

void
RaCompleteStatsCalculator::SetRachDelayFilename(std::string filename)
{
  m_raCompletedFilename = filename;
}

void 
RaCompleteStatsCalculator::SetStartTime (Time t)
{
  m_startTime = t;
  RescheduleEndEpoch ();
}

Time 
RaCompleteStatsCalculator::GetStartTime () const
{
  return m_startTime;
}

void 
RaCompleteStatsCalculator::SetEpoch (Time e)
{
  m_epochDuration = e;
  RescheduleEndEpoch ();
}

Time 
RaCompleteStatsCalculator::GetEpoch () const
{
  return m_epochDuration;  
}

void
RaCompleteStatsCalculator::StorePreambleTx(uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	//NS_LOG_ERROR("StorePreambleTx");
	// this method will store every tx attempt on prach for each imsi
	ImsiRntiCidTimeInfo_t info;
	info.m_time = Simulator::Now();
	info.m_imsi = imsi;
	info.m_cellId = cellId;
	info.m_rnti = rnti; // probably useless
	ImsiInfoMap_t::iterator it = m_preambleTxEvents.find(imsi);
	if (it == m_preambleTxEvents.end())
	{
		ImsiRntiCidTimeInfoList_t list;
		list.push_back(info);
		m_preambleTxEvents[imsi] = list;
	}
	else
	{
		it->second.push_back(info);
	}
	//NS_LOG_ERROR("StorePreambleTx for imsi " << imsi << " at time " << Simulator::Now().GetSeconds());
}

void
RaCompleteStatsCalculator::StorePreambleTx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	NS_LOG_FUNCTION (raStats << path);
  	raStats->StorePreambleTx (imsi, cellId, rnti);
}

void
RaCompleteStatsCalculator::StoreMsg3Tx(uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	// this method will store every tx attempt of msg3 for each imsi
	ImsiRntiCidTimeInfo_t info;
	info.m_time = Simulator::Now();
	info.m_imsi = imsi;
	info.m_cellId = cellId;
	info.m_rnti = rnti; // probably useless
	ImsiInfoMap_t::iterator it = m_msg3TxEvents.find(imsi);
	if (it == m_msg3TxEvents.end())
	{
		ImsiRntiCidTimeInfoList_t list;
		list.push_back(info);
		m_msg3TxEvents[imsi] = list;
	}
	else
	{
		it->second.push_back(info);
	}
}

void
RaCompleteStatsCalculator::StoreMsg3Tx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	NS_LOG_FUNCTION (raStats << path);
  	raStats->StoreMsg3Tx (imsi, cellId, rnti);
}

void
RaCompleteStatsCalculator::StoreMsg4Rx(uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	//NS_LOG_ERROR("StoreMsg4Rx for imsi " << imsi << " at time " << Simulator::Now().GetSeconds());
	// this method will store every tx attempt on prach for each imsi
	ImsiRntiCidTimeInfo_t info;
	info.m_time = Simulator::Now();
	info.m_imsi = imsi;
	info.m_cellId = cellId;
	info.m_rnti = rnti;
	ImsiInfoMap_t::iterator it = m_msg4RxEvents.find(imsi);
	if (it == m_msg4RxEvents.end())
	{
		ImsiRntiCidTimeInfoList_t list;
		list.push_back(info);
		m_msg4RxEvents[imsi] = list;
	}
	else
	{
		it->second.push_back(info);
	}

	// compute total delay
	ImsiInfoMap_t::iterator preambleTxIterator = m_preambleTxEvents.find(imsi);
	NS_ASSERT_MSG(preambleTxIterator != m_preambleTxEvents.end(), "Imsi never inserted");
	preambleTxIterator->second.sort();
	//NS_LOG_ERROR("Sorted, get list iterator");
	//NS_LOG_ERROR("Size of preambleTxIterator associated list " << preambleTxIterator->second.size());

	ImsiRntiCidTimeInfoList_t::iterator listIterator = preambleTxIterator->second.begin();
	while(listIterator->m_cellId != cellId) // get the first preamble tx for this cellId
		{
			listIterator++;	
			//NS_LOG_ERROR("listIterator " << listIterator->m_time.GetSeconds());
		}
	Time delay = Simulator::Now() - listIterator->m_time;
	NS_ASSERT_MSG(listIterator != preambleTxIterator->second.end(), "CellId for this imsi not found");
	//NS_LOG_ERROR("Delay computed " << delay.GetSeconds());
	// delete all other entries related to this (imsi, cellId) pair
	listIterator = preambleTxIterator->second.begin(); 
	while(listIterator != preambleTxIterator->second.end())
		{
			//NS_LOG_ERROR("Cid " << listIterator->m_cellId << " imsi " << listIterator->m_imsi << " time " << listIterator->m_time.GetSeconds());
			if (listIterator->m_cellId == cellId)
				{
					listIterator = preambleTxIterator->second.erase(listIterator);
				}
			else
				{
					++listIterator;
				}	
		}

	ImsiRntiCidTimeInfo_t imsiDlInfo;
	imsiDlInfo.m_imsi = imsi;
	imsiDlInfo.m_time = delay;
	imsiDlInfo.m_cellId = cellId;
	imsiDlInfo.m_rnti = rnti;
	m_imsiDelayMap.insert(std::pair<uint64_t, ImsiRntiCidTimeInfo_t> (imsi, imsiDlInfo));
	m_pendingOutput = true;
}

void
RaCompleteStatsCalculator::StoreMsg4Rx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
	NS_LOG_FUNCTION (raStats << path);
  	raStats->StoreMsg4Rx (imsi, cellId, rnti);
}

void
RaCompleteStatsCalculator::ShowResults (void)
{
	//NS_LOG_ERROR (this << GetRachDelayFilename ().c_str ());
	std::ofstream outFile;

	if (m_firstWriteImsiTime == true)
	{
	  outFile.open (GetRachDelayFilename ().c_str ());
	  if (!outFile.is_open ())
	    {
	      NS_LOG_ERROR ("Can't open file " << GetRachDelayFilename ().c_str ());
	      return;
	    }

	  m_firstWriteImsiTime = false;
	  outFile << "wt\tIMSI\tcellId\tdelay";
	  outFile << std::endl;
	}
	else
	{
	  outFile.open (GetRachDelayFilename ().c_str (),  std::ios_base::app);
	  if (!outFile.is_open ())
	    {
	      NS_LOG_ERROR ("Can't open file " << GetRachDelayFilename ().c_str ());
	      return;
	    }
	}

	WriteResults (outFile);
	m_pendingOutput = false;
}


void
RaCompleteStatsCalculator::WriteResults (std::ofstream& outFile)
{
	double wt = Simulator::Now().GetSeconds();
	//NS_LOG_ERROR (this << "Write results");
	for (ImsiDelayMap_t::iterator it = m_imsiDelayMap.begin (); it != m_imsiDelayMap.end (); ++it)
	{
	  outFile << wt << "\t";
	  outFile << it->first << "\t";
	  outFile << it->second.m_cellId << "\t";
	  outFile << it->second.m_time.GetNanoSeconds() / 1.0e9;
	  outFile << std::endl;
	}

	outFile.close ();
}

void
RaCompleteStatsCalculator::ResetResults (void)
{
  NS_LOG_FUNCTION (this);
  m_imsiDelayMap.erase (m_imsiDelayMap.begin (), m_imsiDelayMap.end ());
}

void
RaCompleteStatsCalculator::RescheduleEndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  m_endEpochEvent.Cancel ();
  NS_ASSERT (Simulator::Now ().GetMilliSeconds () == 0); // below event time assumes this
  m_endEpochEvent = Simulator::Schedule (m_startTime + m_epochDuration, &RaCompleteStatsCalculator::EndEpoch, this);
}

void
RaCompleteStatsCalculator::EndEpoch (void)
{
  NS_LOG_FUNCTION (this);
  ShowResults ();
  ResetResults ();
  m_startTime += m_epochDuration;
  m_endEpochEvent = Simulator::Schedule (m_epochDuration, &RaCompleteStatsCalculator::EndEpoch, this);
}

} //namespace