/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#ifndef RA_PREAMBLE_PHY_STATS_CALCULATOR_H_
#define RA_PREAMBLE_PHY_STATS_CALCULATOR_H_

#include "ns3/lte-common.h"
#include "ns3/uinteger.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include <string>
#include <map>
#include <fstream>

namespace ns3 {

struct RxPhyPreambleInfo_t
{
  	uint16_t m_cellId;
	uint64_t  m_imsi;
  	uint8_t m_correct;
  	uint32_t m_rapId;
  	Time m_delay;
  	bool m_collision;
};


// Container Time and rxPreambleInfo_t
typedef std::multimap< Time, RxPhyPreambleInfo_t > EventListPhy_t;

class RaPreamblePhyStatsCalculator : public Object
{
public:

	/**
	* Class constructor
	*/
	RaPreamblePhyStatsCalculator ();

	/**
	* Class destructor
	*/
	virtual
	~RaPreamblePhyStatsCalculator ();

	// Inherited from ns3::Object
	/**
	*  Register this type.
	*  \return The object TypeId.
	*/
	static TypeId GetTypeId (void);
	void DoDispose ();

	/**
	* Get the name of the file where rach preamble tx will be logged.  
	* @return the name of the file where rach preamble tx will be logged
	*/
	std::string GetPreambleRxFilename (void);

		

  	/**
	* Set the name of the file where rach preamble tx will be logged.  
	* @param the name of the file where rach preamble tx will be logged
	*/
	void SetPreambleRxFilename (std::string outputFilename);

	

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
	* Stores in the event list the tx of a preamble, along with time and imsi.  
	* @param time of the event
	* @param imsi
	*/
	void StorePreambleRx(PhyReceptionStatParameters params);

	/** 
	* trace sink
	* 
	* \param raStatsCalculator 
	* \param path
	* \param imsi 
	* \param PhyReceptionStatParameters params (see lte-common.h)
	*/
  	static void StorePreambleRx (Ptr<RaPreamblePhyStatsCalculator> raStats, std::string path,
                           PhyReceptionStatParameters params);


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


   	std::string m_preambleRxFilename;
   	/*std::string m_preambleRxFilename;
   	std::string m_preambleCollisionsFilename;
	std::string m_msg3TxFilename;
	std::string m_msg3RxFilename;
	std::string m_msg4RxFilename;*/

	Time m_startTime;
	Time m_epochDuration;

	bool m_firstWriteImsiTime;
	bool m_pendingOutput;

	EventListPhy_t m_preambleRxEvents;

  	EventId m_endEpochEvent; //!< Event id for next end epoch event


}; // class

} // namespace ns3

#endif /* RA_PREAMBLE_PHY_STATS_CALCULATOR_H_ */
