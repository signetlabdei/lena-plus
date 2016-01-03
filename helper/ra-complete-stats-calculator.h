/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#ifndef RA_COMPLETE_STATS_CALCULATOR_H_
#define RA_COMPLETE_STATS_CALCULATOR_H_

#include "ns3/lte-common.h"
#include "ns3/uinteger.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include <string>
#include <map>
#include <fstream>

namespace ns3 {

struct ImsiRntiCidTimeInfo_t
{
	uint64_t m_imsi;
	uint16_t m_rnti;
	uint16_t m_cellId;
	Time m_time;

	// TODO define an order based on time!
	// then use a map (imsi, std::list<ImsiRntiCidTimeInfo_t>)

	bool operator<(ImsiRntiCidTimeInfo_t other) const
	{
		return m_time < other.m_time;
	}

};

typedef std::list<ImsiRntiCidTimeInfo_t> ImsiRntiCidTimeInfoList_t;
typedef std::multimap < uint64_t, ImsiRntiCidTimeInfo_t > ImsiDelayMap_t;
typedef std::map < uint64_t, ImsiRntiCidTimeInfoList_t > ImsiInfoMap_t;


class RaCompleteStatsCalculator : public Object
{
public:

	/**
	* Class constructor
	*/
	RaCompleteStatsCalculator ();

	/**
	* Class destructor
	*/
	virtual
	~RaCompleteStatsCalculator ();

	// Inherited from ns3::Object
	/**
	*  Register this type.
	*  \return The object TypeId.
	*/
	static TypeId GetTypeId (void);
	void DoDispose ();

	/**
	* Get the name of the file where the time to complete rach procedure will be logged.  
	* @return the name of the file where the time to complete rach procedure will be logged
	*/
	std::string GetRachDelayFilename (void);

		

  	/**
	* Set the name of the file where the time to complete rach procedure will be logged.  
	* @param the name of the file where the time to complete rach procedure will be logged
	*/
	void SetRachDelayFilename (std::string outputFilename);

	
	/** 
	* 
	* \param t the value of the StartTime attribute
	*/
	void SetStartTime (Time t);

	/** 
	* 
	* \return the value of the StartTime attribute
	*/
	Time GetStartTime () const;

	/** 
	* 
	* \param e the epoch duration
	*/
	void SetEpoch (Time e);

	/** 
	* 
	* \return the epoch duration
	*/
	Time GetEpoch () const;


	/**
	* Stores in the event list the tx of a preamble, along with imsi.  
	* @param imsi
	* @param cellId
	* @param rnti
	*/
	void StorePreambleTx(uint64_t imsi, uint16_t cellId, uint16_t rnti);

	/** 
	* trace sink
	* 
	* \param raStatsCalculator 
	* \param path
	* @param imsi
	* @param cellId
	* @param rnti	
	*/
  	static void StorePreambleTx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti);

  	/**
	* Stores in the event list when a msg3 is queued, along with imsi.  
	* @param imsi
	* @param cellId
	* @param rnti
	*/
	void StoreMsg3Tx(uint64_t imsi, uint16_t cellId, uint16_t rnti);


	/** 
	* trace sink
	* 
	* \param raStatsCalculator 
	* \param path
	* @param imsi
	* @param cellId
	* @param rnti
	*/
  	static void StoreMsg3Tx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti);


  	/**
	* Stores in the event list when a msg4 is received, along with imsi, cellId and rnti.  
	* @param imsi
	* @param cellId
	* @param rnti
	*/
	void StoreMsg4Rx(uint64_t imsi, uint16_t cellId, uint16_t rnti);


	/** 
	* trace sink
	* 
	* @param raStatsCalculator 
	* @param path
	* @param imsi 
	* @param cellId
	* @param rnti
	*/
  	static void StoreMsg4Rx (Ptr<RaCompleteStatsCalculator> raStats, std::string path,
                           uint64_t imsi, uint16_t cellId, uint16_t rnti);




private:

	/**
	* Called after each epoch to write collected
	* statistics to output files. During first call
	* it opens output files and write columns descriptions.
	* During next calls it opens output files in append mode.
	*/
	void
	ShowResults (void);

	/**
	* Writes collected statistics to output file and
	* closes output file.
	* @param outFile ofstream for statistics
	*/
	void
	WriteResults (std::ofstream& outFile);


	/**
	* Erases collected statistics
	*/
	void
	ResetResults (void);

	/**
	* Reschedules EndEpoch event. Usually used after
	* execution of SetStartTime() or SetEpoch()
	*/
	void RescheduleEndEpoch ();

	/**
	* Function called in every endEpochEvent. It calls
	* ShowResults() to write statistics to output files
	* and ResetResults() to clear collected statistics.
	*/
	void EndEpoch (void);


   	std::string m_raCompletedFilename;

	Time m_startTime;
	Time m_epochDuration;

	bool m_firstWriteImsiTime;
	bool m_pendingOutput;

	ImsiInfoMap_t m_preambleTxEvents;
	ImsiInfoMap_t m_msg3TxEvents;
	ImsiInfoMap_t m_msg4RxEvents;
	ImsiDelayMap_t m_imsiDelayMap;

  	EventId m_endEpochEvent; //!< Event id for next end epoch event


}; // class

} // namespace ns3

#endif /* RA_COMPLETE_STATS_CALCULATOR_H_ */
