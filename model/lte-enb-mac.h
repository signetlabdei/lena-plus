/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2015, University of Padova, Dep. of Information Engineering, SIGNET lab.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 *
 * Modified by Michele Polese <michele.polese@gmail.com>
 *    (support for RACH realistic model) 
 */

#ifndef LTE_ENB_MAC_H
#define LTE_ENB_MAC_H


#include <map>
#include <vector>
#include <ns3/lte-common.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/ff-mac-csched-sap.h>
#include <ns3/ff-mac-sched-sap.h>
#include <ns3/lte-enb-phy-sap.h>
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include "ns3/vector.h"


namespace ns3 {

class DlCqiLteControlMessage;
class UlCqiLteControlMessage;
class PdcchMapLteControlMessage;

typedef std::vector <std::vector < Ptr<PacketBurst> > > DlHarqProcessesBuffer_t;
typedef std::multimap<uint8_t, Vector> RapIdPositionMap_t;

/**
 * This class implements the MAC layer of the eNodeB device
 */
class LteEnbMac :   public Object
{
  friend class EnbMacMemberLteEnbCmacSapProvider;
  friend class EnbMacMemberLteMacSapProvider<LteEnbMac>;
  friend class EnbMacMemberFfMacSchedSapUser;
  friend class EnbMacMemberFfMacCschedSapUser;
  friend class EnbMacMemberLteEnbPhySapUser;

public:
  static TypeId GetTypeId (void);

  LteEnbMac (void);
  virtual ~LteEnbMac (void);
  virtual void DoDispose (void);


  /**
   * \brief Set the scheduler SAP provider
   * \param s a pointer SAP provider of the FF packet scheduler
   */
  void SetFfMacSchedSapProvider (FfMacSchedSapProvider* s);
  /**
   * \brief Get the scheduler SAP user
   * \return a pointer to the SAP user of the scheduler
   */
  FfMacSchedSapUser* GetFfMacSchedSapUser (void);
  /**
   * \brief Set the control scheduler SAP provider
   * \param s a pointer to the control scheduler SAP provider
   */
  void SetFfMacCschedSapProvider (FfMacCschedSapProvider* s);
  /**
   * \brief Get the control scheduler SAP user
   * \return a pointer to the control scheduler SAP user
   */
  FfMacCschedSapUser* GetFfMacCschedSapUser (void);



  /**
   * \brief Set the MAC SAP user
   * \param s a pointer to the MAC SAP user
   */
  void SetLteMacSapUser (LteMacSapUser* s);
  /**
   * \brief Get the MAC SAP provider
   * \return a pointer to the SAP provider of the MAC
   */
  LteMacSapProvider* GetLteMacSapProvider (void);
  /**
   * \brief Set the control MAC SAP user
   * \param s a pointer to the control MAC SAP user
   */
  void SetLteEnbCmacSapUser (LteEnbCmacSapUser* s);
  /**
   * \brief Get the control MAC SAP provider
   * \return a pointer to the control MAC SAP provider
   */
  LteEnbCmacSapProvider* GetLteEnbCmacSapProvider (void);


  /**
  * \brief Get the eNB-PHY SAP User
  * \return a pointer to the SAP User of the PHY
  */
  LteEnbPhySapUser* GetLteEnbPhySapUser ();

  /**
  * \brief Set the PHY SAP Provider
  * \param s a pointer to the PHY SAP provider
  */
  void SetLteEnbPhySapProvider (LteEnbPhySapProvider* s);

  /**
   * Set and Get for PRACH ideal or real,
   * used by LteHelper
   */
  void SetPrachMode(bool ideal);
  bool GetPrachMode() const;

  /**
   * TracedCallback signature for DL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs0 The MCS for transport block.. 
   * \param [in] tbs0Size
   * \param [in] mcs1 The MCS for transport block.
   * \param [in] tbs1Size
   */
  typedef void (* DlSchedulingTracedCallback)
    (uint32_t frame, uint32_t subframe,  uint16_t rnti,
     uint8_t mcs0, uint16_t tbs0Size,
     uint8_t mcs1, uint16_t tbs1Size);

  /**
   *  TracedCallback signature for UL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs  The MCS for transport block
   * \param [in] tbsSize
   */
  typedef void (* UlSchedulingTracedCallback)
    (uint32_t frame, uint32_t subframe, uint16_t rnti,
     uint8_t mcs, uint16_t tbsSize);

  /**
   *  TracedCallback signature for rach preamble rx events.
   *
   * \param [in] RachPreambleLteControlMessage msg
   */
  typedef void (* RachPreambleReceivedTracedCallback)
    (const Ptr<RachPreambleLteControlMessage> msg);


  /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);
  
private:

  /**
  * \brief Receive a DL CQI ideal control message
  * \param msg the DL CQI message
  */
  void ReceiveDlCqiLteControlMessage  (Ptr<DlCqiLteControlMessage> msg);

  void DoReceiveLteControlMessage (Ptr<LteControlMessage> msg);

  /**
  * \brief Receive a CE element containing the buffer status report
  * \param bsr the BSR message
  */
  void ReceiveBsrMessage  (MacCeListElement_s bsr);

 
  void DoUlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);



  // forwarded from LteEnbCmacSapProvider
  void DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser* msu);
  void DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo);
  void DoReleaseLc (uint16_t rnti);
  void DoReleaseLc (uint16_t  rnti, uint8_t lcid);
  void DoUeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params);
  LteEnbCmacSapProvider::RachConfig DoGetRachConfig ();
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue DoAllocateNcRaPreamble (uint16_t rnti);

  // forwarded from LteMacSapProvider
  void DoTransmitPdu (LteMacSapProvider::TransmitPduParameters);
  void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters);


  // forwarded from FfMacCchedSapUser
  void DoCschedCellConfigCnf (FfMacCschedSapUser::CschedCellConfigCnfParameters params);
  void DoCschedUeConfigCnf (FfMacCschedSapUser::CschedUeConfigCnfParameters params);
  void DoCschedLcConfigCnf (FfMacCschedSapUser::CschedLcConfigCnfParameters params);
  void DoCschedLcReleaseCnf (FfMacCschedSapUser::CschedLcReleaseCnfParameters params);
  void DoCschedUeReleaseCnf (FfMacCschedSapUser::CschedUeReleaseCnfParameters params);
  void DoCschedUeConfigUpdateInd (FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  void DoCschedCellConfigUpdateInd (FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params);

  // forwarded from FfMacSchedSapUser
  void DoSchedDlConfigInd (FfMacSchedSapUser::SchedDlConfigIndParameters ind);
  void DoSchedUlConfigInd (FfMacSchedSapUser::SchedUlConfigIndParameters params);

  // forwarded from LteEnbPhySapUser
  void DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  void DoReceiveRachPreamble (Ptr<RachPreambleLteControlMessage> msg);

public:
  // legacy public for use the Phy callback
  void DoReceivePhyPdu (Ptr<Packet> p);

private:
  void DoUlInfoListElementHarqFeeback (UlInfoListElement_s params);
  void DoDlInfoListElementHarqFeeback (DlInfoListElement_s params);

  /**
   *  \params the list with vectors representing UEpos - eNBpos
   */ 
  bool EvaluateCollisionProbability(std::list<Vector> pointList);

  //            rnti,             lcid, SAP of the RLC instance
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> > m_rlcAttached;

  std::vector <CqiListElement_s> m_dlCqiReceived; // DL-CQI received
  std::vector <FfMacSchedSapProvider::SchedUlCqiInfoReqParameters> m_ulCqiReceived; // UL-CQI received
  std::vector <MacCeListElement_s> m_ulCeReceived; // CE received (BSR up to now)

  std::vector <DlInfoListElement_s> m_dlInfoListReceived; // DL HARQ feedback received

  std::vector <UlInfoListElement_s> m_ulInfoListReceived; // UL HARQ feedback received

  bool m_message3Received;
  /*
  * Map of UE's info element (see 4.3.12 of FF MAC Scheduler API)
  */
//   std::map <uint16_t,UlInfoListElement_s> m_ulInfoListElements; 



  LteMacSapProvider* m_macSapProvider;
  LteEnbCmacSapProvider* m_cmacSapProvider;
  LteMacSapUser* m_macSapUser;
  LteEnbCmacSapUser* m_cmacSapUser;


  FfMacSchedSapProvider* m_schedSapProvider;
  FfMacCschedSapProvider* m_cschedSapProvider;
  FfMacSchedSapUser* m_schedSapUser;
  FfMacCschedSapUser* m_cschedSapUser;

  // PHY-SAP
  LteEnbPhySapProvider* m_enbPhySapProvider;
  LteEnbPhySapUser* m_enbPhySapUser;

  uint32_t m_frameNo;
  uint32_t m_subframeNo;
  /**
   * Trace information regarding DL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB1, size of TB1,
   * MCS of TB2 (0 if not present), size of TB2 (0 if not present)
   */
  TracedCallback<uint32_t, uint32_t, uint16_t,
                uint8_t, uint16_t, uint8_t, uint16_t> m_dlScheduling;

  /**
   * Trace information regarding UL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB, size of TB
   */
  TracedCallback<uint32_t, uint32_t, uint16_t,
                 uint8_t, uint16_t> m_ulScheduling;  

  /**
   * Trace information regarding rach preamble rx 
   */
  TracedCallback<Ptr<RachPreambleLteControlMessage> > m_receiveRachPreambleTrace;
  
  uint8_t m_macChTtiDelay; // delay of MAC, PHY and channel in terms of TTIs


  std::map <uint16_t, DlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets; // Packet under trasmission of the DL HARQ process
  
  uint8_t m_numberOfRaPreambles;
  uint8_t m_preambleTransMax;
  uint8_t m_raResponseWindowSize;
  uint8_t m_pRachConfigurationIndex;
  uint8_t m_powerRampingStep;
  uint8_t m_preambleInitialReceivedTargetPower;
  uint8_t m_contentionResolutionTimer;
  uint8_t m_backoffIndicator;
  bool m_realPrach;

  /**
   * info associated with a preamble allocated for non-contention based RA
   * 
   */
  struct NcRaPreambleInfo
  {   
    uint16_t rnti; ///< rnti previously allocated for this non-contention based RA procedure
    Time expiryTime; ///< value the expiration time of this allocation (so that stale preambles can be reused)
  };

  /**
   * map storing as key the random acccess preamble IDs allocated for
   * non-contention based access, and as value the associated info
   * 
   */
  std::map<uint8_t, NcRaPreambleInfo> m_allocatedNcRaPreambleMap;
 
  // stores rapId and node position (internal hack to evaluate if there can be a collision at the preamble)
  RapIdPositionMap_t m_receivedRachPreambleCount;

  std::map<uint8_t, uint32_t> m_rapIdRntiMap;
  Ptr<UniformRandomVariable> m_random;
};

} // end namespace ns3

#endif /* LTE_ENB_MAC_ENTITY_H */
