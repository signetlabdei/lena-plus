/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#ifndef RA_PREAMBLE_STATS_CALCULATOR_H_
#define RA_PREAMBLE_STATS_CALCULATOR_H_

#include "ns3/lte-common.h"
#include "ns3/uinteger.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include <string>
#include <map>
#include <fstream>
#include "ns3/lte-control-messages.h"
#include "ra-preamble-phy-stats-calculator.h"

namespace ns3 {

struct RxPreambleInfo_t
{
	uint64_t m_imsi;
	double m_delay;
	uint32_t m_rapId;
	bool m_collision; // true: there is a collision
};

// Container Time and rxPreambleInfo_t
typedef std::multimap< Time, RxPreambleInfo_t > EventList_t;
typedef std::pair< Time, uint64_t > TimeImsiPair_t;
typedef std::multimap < TimeImsiPair_t, RxPhyPreambleInfo_t > EventPhyList_t;

class RaPreambleStatsCalculator : public Object
{
public:

	/**
	* Class constructor
	*/
	RaPreambleStatsCalculator ();

	/**
	* Class destructor
	*/
	virtual
	~RaPreambleStatsCalculator ();

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
	* @param rach preamble msg
	*/
	void StorePreambleRx(Ptr<RachPreambleLteControlMessage> msg);

	/** 
	* trace sink
	* 
	* \param raStatsCalculator 
	* \param path
	* \param rach preamble msg
	*/
  	static void StorePreambleRx (Ptr<RaPreambleStatsCalculator> raStats, std::string path,
                           Ptr<RachPreambleLteControlMessage> msg);

  	/**
	* Stores in the event list the rx of a preamble, along with time and imsi.  
	* @param the PhyReceptionStatParameters
	*/
	void StorePreamblePhyRx(PhyReceptionStatParameters params);


	/** 
	* trace sink
	* 
	* \param raStatsCalculator 
	* \param path
	* \param imsi 
	* \param PhyReceptionStatParameters params (see lte-common.h)
	*/
  	static void StorePreamblePhyRx (Ptr<RaPreambleStatsCalculator> raStats, std::string path,
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

	Time m_startTime;
	Time m_epochDuration;

	bool m_firstWriteImsiTime;
	bool m_pendingOutput;

	EventList_t m_preambleRxEvents;
	EventPhyList_t m_preamblePhyRxEvents;

  	EventId m_endEpochEvent; //!< Event id for next end epoch event


}; // class

} // namespace ns3

#endif /* RA_PREAMBLE_STATS_CALCULATOR_H_ */
