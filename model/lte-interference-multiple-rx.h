/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by Michele Polese <michele.polese@gmail.com>
 *    (support for RACH realistic model: reception of multiple PRACH preambles)
 */


#ifndef LTE_INTERFERENCE_MULTIPLE_RX_H
#define LTE_INTERFERENCE_MULTIPLE_RX_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-value.h>
#include "lte-chunk-processor-multiple.h"

#include <list>
#include "map"

namespace ns3 {


/**
 * This class implements a gaussian interference model, i.e., all
 * incoming signals are added to the total interference. Moreover 
 * if more than one signal on the same rbs is received at a time, 
 * it handles the correct sinr computation.
 *
 */
class LteInterferenceMultipleRx : public Object
{
public:
  LteInterferenceMultipleRx ();
  virtual ~LteInterferenceMultipleRx ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Add a LteChunkProcessor that will use the time-vs-frequency SINR
   * calculated by this LteInterferenceMultipleRx instance. Note that all the
   * added LteChunkProcessors will work in parallel.
   *
   * @param p
   */
  void AddSinrChunkProcessor (Ptr<LteChunkProcessorMultiple> p);

  /**
   * Add a LteChunkProcessor that will use the time-vs-frequency
   *  interference calculated by this LteInterferenceMultipleRx instance. Note 
   *  that all the added LteChunkProcessors will work in parallel.
   *
   * @param p
   */
  void AddInterferenceChunkProcessor (Ptr<LteChunkProcessorMultiple> p);

  /**
   * Add a LteChunkProcessor that will use the time-vs-frequency
   *  power calculated by this LteInterferenceMultipleRx instance. Note
   *  that all the added LteChunkProcessors will work in parallel.
   *
   * @param p
   */
  void AddRsPowerChunkProcessor (Ptr<LteChunkProcessorMultiple> p);

  /**
   * notify that the PHY is starting a RX attempt on signal signalId
   *
   * @param rxPsd the power spectral density of the signal being RX
   * @param signalId the unique identifier of the signal 
   */
  void StartRx (Ptr<const SpectrumValue> rxPsd, uint32_t signalId);


  /**
   * notify that the RX attempt has ended for signal signalId. 
   * The receiving PHY must call this method when RX ends or RX is aborted.
   *
   * @param signalId the unique identifier of the signal
   */
  void EndRx (uint32_t signalId);


  /**
   * notify that all the simultaneous RX have ended

   */
  void EndAllRx (void);


  /**
   * notify that a new signal is being perceived in the medium. This
   * method is to be called for all incoming signal, regardless of
   * wether they're useful signals or interferers.
   *
   * @param spd the power spectral density of the new signal
   * @param duration the duration of the new signal
   * @param signalId the unique identifier of the signal
   */
  void AddSignal (Ptr<const SpectrumValue> spd, const Time duration);


  /**
   *
   * @param noisePsd the Noise Power Spectral Density in power units
   * (Watt, Pascal...) per Hz.
   */
  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);

private:
  void ConditionallyEvaluateChunk (uint32_t signalId);
  void DoAddSignal  (Ptr<const SpectrumValue> spd);
  // void DoSubtractSignal  (Ptr<const SpectrumValue> spd, uint32_t internal_signalId, uint32_t signalId);

  std::map < uint32_t, bool> m_receivingMap;

  std::map < uint32_t, Ptr<SpectrumValue> > m_rxSignalMap; 
                                  /**< stores the power spectral density of
                                  * the signals whose RX is being
                                  * attempted
                                  */

  Ptr<SpectrumValue> m_allSignals; /**< stores the spectral
                                    * power density of the sum of incoming signals;
                                    * does not include noise, includes the SPD of the signal being RX
                                    */

  Ptr<const SpectrumValue> m_noise;

  std::map<uint32_t, Time > m_lastChangeTimeMap;     
                                /**< the time of the last change in
                                m_TotalPower for each signal */

  uint32_t m_lastSignalId;
  uint32_t m_lastSignalIdBeforeReset;

  /** all the processor instances that need to be notified whenever
  a new interference chunk is calculated */
  std::list<Ptr<LteChunkProcessorMultiple> > m_rsPowerChunkProcessorList;

  /** all the processor instances that need to be notified whenever
      a new SINR chunk is calculated */
  std::list<Ptr<LteChunkProcessorMultiple> > m_sinrChunkProcessorList;

  /** all the processor instances that need to be notified whenever
      a new interference chunk is calculated */
  std::list<Ptr<LteChunkProcessorMultiple> > m_interfChunkProcessorList;


};



} // namespace ns3





#endif /* LTE_INTERFERENCE_H */
