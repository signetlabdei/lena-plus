/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Michele Polese <michele.polese@gmail.com> 
 */

#ifndef EPC_S1AP_H
#define EPC_S1AP_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

#include "ns3/epc-s1ap-sap.h"

#include <map>

namespace ns3 {


class S1apIfaceInfo : public SimpleRefCount<S1apIfaceInfo>
{
public:
  S1apIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket);
  virtual ~S1apIfaceInfo (void);

  S1apIfaceInfo& operator= (const S1apIfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr;
  Ptr<Socket>   m_localCtrlPlaneSocket;
};


class S1apConnectionInfo : public SimpleRefCount<S1apConnectionInfo>
{
public:
  S1apConnectionInfo (uint16_t enbId, uint16_t mmeId);
  virtual ~S1apConnectionInfo (void);

  S1apConnectionInfo& operator= (const S1apConnectionInfo &);

public:
  uint16_t m_enbId;
  uint16_t m_mmeId;
};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the S1AP interface
 */
class EpcS1apEnb : public Object
{
  friend class MemberEpcS1apSapEnbProvider<EpcS1apEnb>;

public:
  /** 
   * Constructor
   */
  EpcS1apEnb ();

  /**
   * Destructor
   */
  virtual ~EpcS1apEnb (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the S1ap SAP User to be used by this EPC S1ap eNB entity in order to call methods of epcEnbApplication
   */
  void SetEpcS1apSapEnbUser (EpcS1apSapEnb * s);

  /**
   * \return the S1ap SAP Provider interface offered by this EPC S1ap entity
   */
  EpcS1apSapEnbProvider* GetEpcS1apSapEnbProvider ();


  /**
   * Add an S1ap interface to this EPC S1ap entity
   * \param enbId the cell ID of the eNodeB on which this is installed
   * \param enbAddress the address of the eNodeB on which this is installed
   * \param mmeId the ID of the MME to which the eNB is connected
   * \param mmeAddress the address of the MME to which the eNB is connected
   */
  void AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t mmeId, Ipv4Address mmeAddress);


  /** 
   * Method to be assigned to the recv callback of the S1ap-C (S1ap Control Plane) socket.
   * It is called when the eNB receives a packet from the MME on the S1ap interface
   * 
   * \param socket socket of the S1ap interface
   */
  void RecvFromS1apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by EpcS1apSapEnbProvider
  virtual void DoSendInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t stmsi, uint16_t ecgi) = 0;
  virtual void DoSendErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication ) = 0;
  virtual void DoSendInitialContextSetupResponse (uint64_t mmeUeS1Id,
                                                  uint16_t enbUeS1Id,
                                                  std::list<EpcS1apSap::ErabSetupItem> erabSetupList) = 0;
  virtual void DoSendPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<EpcS1apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList) = 0;

  EpcS1apSapEnb* m_s1apSapUser;
  EpcS1apSapEnbProvider* m_s1apSapProvider;


private:

  /**
   * Map the mmeId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the S1ap message
   */
  std::map < uint16_t, Ptr<S1apIfaceInfo> > m_s1apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the S1ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the S1ap interface
   */
  std::map < Ptr<Socket>, Ptr<S1apConnectionInfo> > m_s1apInterfaceCellIds;

  /**
   * UDP port to be used for the S1ap interfaces: S1ap
   */
  uint16_t m_s1apUdpPort;

  /**
   * Mme ID, stored as a private variable until more than one MME will be implemented
   */
  uint16_t m_mmeId; 

};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the S1AP interface
 */
class EpcS1apMme : public Object
{
  friend class MemberEpcS1apSapMmeProvider<EpcS1apMme>;

public:
  /** 
   * Constructor
   */
  EpcS1apMme ();

  /**
   * Destructor
   */
  virtual ~EpcS1apMme (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the S1ap SAP User to be used by this EPC S1ap Mme entity in order to call methods of epcMmeApplication
   */
  void SetEpcS1apSapMmeUser (EpcS1apSapMme * s);

  /**
   * \return the S1ap SAP Provider interface offered by this EPC S1ap entity
   */
  EpcS1apSapMmeProvider* GetEpcS1apSapMmeProvider ();


  /**
   * Add an S1ap interface to this EPC S1ap entity
   * \param enbId the cell ID of the eNodeB which the MME is connected to
   * \param enbAddress the address of the eNodeB which the MME is connected to
   * \param mmeId the ID of the MME on which this entity is installed
   * \param mmeAddress the address of the MME on which this entity is installed
   */
  void AddS1apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t mmeId, Ipv4Address mmeAddress);


  /** 
   * Method to be assigned to the recv callback of the S1ap-C (S1ap Control Plane) socket.
   * It is called when the MME receives a packet from the eNB on the S1ap interface
   * 
   * \param socket socket of the S1ap interface
   */
  void RecvFromS1apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by EpcS1apSapMmeProvider
  virtual void DoSendInitialContextSetupRequest (uint64_t mmeUeS1Id,
                                           uint16_t enbUeS1Id,
                                           std::list<EpcS1apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId);

  virtual void DoSendPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi, 
                                        std::list<EpcS1apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);

  EpcS1apSapMme* m_s1apSapUser;
  EpcS1apSapMmeProvider* m_s1apSapProvider;


private:

  /**
   * Map the enbId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the S1ap message
   */
  std::map < uint16_t, Ptr<S1apIfaceInfo> > m_s1apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the S1ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the S1ap interface
   */
  std::map < Ptr<Socket>, Ptr<S1apConnectionInfo> > m_s1apInterfaceCellIds;

  /**
   * UDP port to be used for the S1ap interfaces: S1ap
   */
  uint16_t m_s1apUdpPort;

};

} //namespace ns3

#endif // EPC_S1AP_H
