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
 * This is based on LteInterference class by
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by Michele Polese <michele.polese@gmail.com>
 *    (support for RACH realistic model: reception of multiple PRACH preambles)
 */


#include "lte-interference-multiple-rx.h"
#include "lte-chunk-processor-multiple.h"

#include <ns3/simulator.h>
#include <ns3/log.h>
#include "map"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteInterferenceMultipleRx");

LteInterferenceMultipleRx::LteInterferenceMultipleRx ()
  : m_lastSignalId (0),
    m_lastSignalIdBeforeReset (0)
{
  NS_LOG_FUNCTION (this);
}

LteInterferenceMultipleRx::~LteInterferenceMultipleRx ()
{
  NS_LOG_FUNCTION (this);
}

void 
LteInterferenceMultipleRx::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rsPowerChunkProcessorList.clear ();
  m_sinrChunkProcessorList.clear ();
  m_interfChunkProcessorList.clear ();
  m_rxSignalMap.clear();
  m_allSignals = 0;
  m_noise = 0;
  Object::DoDispose ();
} 


TypeId
LteInterferenceMultipleRx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteInterferenceMultipleRx")
    .SetParent<Object> ()
    .SetGroupName("Lte")
  ;
  return tid;
}


void
LteInterferenceMultipleRx::StartRx (Ptr<const SpectrumValue> rxPsd, uint32_t signalId)
{ 
  NS_LOG_FUNCTION (this << *rxPsd);
  m_rxSignalMap[signalId] = rxPsd->Copy ();
  m_lastChangeTimeMap[signalId] = Now ();
  m_receivingMap[signalId] = true;
  for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
    {
      (*it)->Start (signalId);
    }
  for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
    {
      (*it)->Start (signalId);
    }
  for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
    {
      (*it)->Start (signalId); 
    }
    
}


void
LteInterferenceMultipleRx::EndRx (uint32_t signalId)
{
  NS_LOG_FUNCTION (this);
  std::map< uint32_t, bool >::iterator boolIterator = m_receivingMap.find(signalId);
  NS_ASSERT(boolIterator != m_receivingMap.end());  
  if (boolIterator->second != true)
    {
      NS_LOG_INFO ("EndRx was already evaluated or RX was aborted");
    }
  else
    {
      ConditionallyEvaluateChunk (signalId);
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->End (signalId);
        }
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->End (signalId);
        }
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->End (signalId); 
        }
    }
}


void
LteInterferenceMultipleRx::EndAllRx()
{
  m_receivingMap.clear();
  m_rxSignalMap.clear();
  m_lastChangeTimeMap.clear();
  (*m_allSignals) -= (*m_allSignals);
}



void
LteInterferenceMultipleRx::AddSignal (Ptr<const SpectrumValue> spd, const Time duration)
{
  NS_LOG_FUNCTION (this << *spd << duration);
  DoAddSignal (spd);
  uint32_t signalNumber = ++m_lastSignalId;
  if (signalNumber == m_lastSignalIdBeforeReset)
    {
      // This happens when m_lastSignalId eventually wraps around. Given that so
      // many signals have elapsed since the last reset, we hope that by now there is
      // no stale pending signal (i.e., a signal that was scheduled
      // for subtraction before the reset). So we just move the
      // boundary further.
      m_lastSignalIdBeforeReset += 0x10000000;
    }
  // Simulator::Schedule (duration, &LteInterferenceMultipleRx::DoSubtractSignal, this, spd, signalNumber);
}


void
LteInterferenceMultipleRx::DoAddSignal  (Ptr<const SpectrumValue> spd)
{ 
  NS_LOG_FUNCTION (this << *spd);
  // ConditionallyEvaluateChunk ();
  (*m_allSignals) += (*spd);
}


void
LteInterferenceMultipleRx::ConditionallyEvaluateChunk (uint32_t signalId)
{
  NS_LOG_FUNCTION (this);
  std::map< uint32_t, bool >::iterator boolIterator = m_receivingMap.find(signalId);
  NS_ASSERT(boolIterator != m_receivingMap.end());  
  if (boolIterator->second == true)
    {
      NS_LOG_DEBUG (this << " Receiving");
    }
  std::map< uint32_t, Time >::iterator timeIterator = m_lastChangeTimeMap.find(signalId);
  NS_ASSERT(timeIterator != m_lastChangeTimeMap.end());
  NS_LOG_DEBUG (this << " now "  << Now () << " last " << timeIterator->second);
  Ptr<SpectrumValue> rxSignal = m_rxSignalMap.find(signalId)->second;
  if (boolIterator->second && Now() > timeIterator->second) 
    {
      NS_LOG_LOGIC (this << " signal = " << *rxSignal << " allSignals = " << *m_allSignals << " noise = " << *m_noise);

      SpectrumValue interf =  (*m_allSignals) - (*rxSignal) + (*m_noise);

      SpectrumValue snr = (*rxSignal) / (*m_noise);
      Time duration = Now () - timeIterator->second;
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (snr, duration, signalId);
        }
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_interfChunkProcessorList.begin (); it != m_interfChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (interf, duration, signalId);
        }
      for (std::list<Ptr<LteChunkProcessorMultiple> >::const_iterator it = m_rsPowerChunkProcessorList.begin (); it != m_rsPowerChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (*rxSignal, duration, signalId);
        }
      timeIterator->second = Now ();
    }
}

void
LteInterferenceMultipleRx::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << *noisePsd);
  m_noise = noisePsd;
  // reset m_allSignals (will reset if already set previously)
  // this is needed since this method can potentially change the SpectrumModel
  m_allSignals = Create<SpectrumValue> (noisePsd->GetSpectrumModel ());
  m_receivingMap.clear();
  // record the last SignalId so that we can ignore all signals that
  // were scheduled for subtraction before m_allSignals 
  m_lastSignalIdBeforeReset = m_lastSignalId;
}

void
LteInterferenceMultipleRx::AddRsPowerChunkProcessor (Ptr<LteChunkProcessorMultiple> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rsPowerChunkProcessorList.push_back (p);
}

void
LteInterferenceMultipleRx::AddSinrChunkProcessor (Ptr<LteChunkProcessorMultiple> p)
{
  NS_LOG_FUNCTION (this << p);
  m_sinrChunkProcessorList.push_back (p);
}

void
LteInterferenceMultipleRx::AddInterferenceChunkProcessor (Ptr<LteChunkProcessorMultiple> p)
{
  NS_LOG_FUNCTION (this << p);
  m_interfChunkProcessorList.push_back (p);
}




} // namespace ns3


