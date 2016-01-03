/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * This is based on lte-chunck-processor.cc by
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by : Marco Miozzo <mmiozzo@cttc.es>
 *        (move from CQI to Ctrl and Data SINR Chunk processors
 *
 * 
 * Modified by : Michele Polese <michele.polese@gmail.com>
 *        (multiple signal support)
 */


#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include "lte-chunk-processor-multiple.h"
#include <ns3/simulator.h>
#include "map"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteChunkProcessorMultiple");

LteChunkProcessorMultiple::LteChunkProcessorMultiple ()
{
  NS_LOG_FUNCTION (this);
}

LteChunkProcessorMultiple::~LteChunkProcessorMultiple ()
{
  NS_LOG_FUNCTION (this);
}

void
LteChunkProcessorMultiple::AddCallback (LteChunkProcessorMultipleCallback c)
{
  NS_LOG_FUNCTION (this);
  m_LteChunkProcessorMultipleCallbacks.push_back (c);
}

void
LteChunkProcessorMultiple::Start (uint32_t signalId)
{
  // clear internal variables
  NS_LOG_FUNCTION (this);
  std::map<uint32_t, Ptr<SpectrumValue> >::iterator spectrumIt = m_sumValuesMap.find(signalId);
  if (spectrumIt == m_sumValuesMap.end())
    {
      m_sumValuesMap.insert(std::pair<uint32_t, Ptr<SpectrumValue> >( signalId, 0 ));
    }
  else
    {
      spectrumIt->second = 0; // release the pointer
    }
  
  std::map<uint32_t, Time >::iterator timeIt = m_totDurationMap.find(signalId);
  if (timeIt == m_totDurationMap.end())
    {
      m_totDurationMap.insert(std::pair<uint32_t, Time >( signalId, MicroSeconds (0) ) );
    }
  else
    {
      timeIt->second = MicroSeconds (0); // set duration to 0
    }
  
}


void
LteChunkProcessorMultiple::EvaluateChunk (const SpectrumValue& sinr, Time duration, uint32_t signalId)
{
  NS_LOG_FUNCTION (this << sinr << duration);
  std::map<uint32_t, Ptr<SpectrumValue> >::iterator spectrumIt = m_sumValuesMap.find(signalId);
  NS_ASSERT(spectrumIt != m_sumValuesMap.end());
  if (spectrumIt->second == 0)
    {
      spectrumIt->second = Create<SpectrumValue> (sinr.GetSpectrumModel ()); 
      // insert a valid pointer
    }
  // update with sinr  
  (*(spectrumIt->second)) += sinr * duration.GetSeconds ();
  
  // update total duration
  std::map<uint32_t, Time >::iterator timeIt = m_totDurationMap.find(signalId);
  NS_ASSERT(timeIt != m_totDurationMap.end());
  timeIt->second += duration;
}


void
LteChunkProcessorMultiple::End (uint32_t signalId)
{
  NS_LOG_FUNCTION (this);
  std::map<uint32_t, Ptr<SpectrumValue> >::iterator spectrumIt = m_sumValuesMap.find(signalId);
  NS_ASSERT(spectrumIt != m_sumValuesMap.end());
  std::map<uint32_t, Time >::iterator timeIt = m_totDurationMap.find(signalId);
  NS_ASSERT(timeIt != m_totDurationMap.end());
  if (timeIt->second.GetSeconds () > 0)
    {

      std::vector<LteChunkProcessorMultipleCallback>::iterator it;
      for (it = m_LteChunkProcessorMultipleCallbacks.begin (); it != m_LteChunkProcessorMultipleCallbacks.end (); it++)
        {
          NS_LOG_LOGIC("Sinr in LteChunkProcessorMultiple " << (*(spectrumIt->second)) / timeIt->second.GetSeconds ());
          (*it)(((*(spectrumIt->second)) / timeIt->second.GetSeconds ()), signalId);
        }
    }
  else
    {
      NS_LOG_WARN ("m_numSinr == 0");
    }
}


  
void
LteSpectrumValueCatcherMultiple::ReportValue (const SpectrumValue& value)
{
  m_value = value.Copy ();
}

Ptr<SpectrumValue> 
LteSpectrumValueCatcherMultiple::GetValue ()
{
  return m_value;
}


} // namespace ns3
