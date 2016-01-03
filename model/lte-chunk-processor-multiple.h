/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009, 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * This is based on lte-chunck-processor.h by
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by : Marco Miozzo <mmiozzo@cttc.es>
 *        (move from CQI to Ctrl and Data SINR Chunk processors)
 * Modified by : Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *        (removed all Lte***ChunkProcessor implementations
 *        and created generic LteChunkProcessor)
 *
 * Modified by : Michele Polese <michele.polese@gmail.com>
 *        (multiple signal support)
 */


#ifndef LTE_CHUNK_PROCESSOR_MULTIPLE_H
#define LTE_CHUNK_PROCESSOR_MULTIPLE_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include "map"

namespace ns3 {

class SpectrumValue;

typedef Callback< void, const SpectrumValue&, uint32_t > LteChunkProcessorMultipleCallback;

/** 
 * This abstract class is used to process the time-vs-frequency
 * SINR/interference/power chunk of a received LTE signal
 * which was calculated by the LteInterference object and it is 
 * an extension of LteChunkProcessorMultiple to support multiple rxs.
 */
class LteChunkProcessorMultiple : public SimpleRefCount<LteChunkProcessorMultiple>
{
public:
  LteChunkProcessorMultiple ();
  virtual ~LteChunkProcessorMultiple ();

  /**
    * \brief Add callback to list
    *
    * This function adds callback c to list. Each callback pass
    * calculated value to its object and is called in
    * LteChunkProcessorMultiple::End().
    */
  virtual void AddCallback (LteChunkProcessorMultipleCallback c);

  /**
    * \brief Clear internal variables
    *
    * This function clears internal variables in the beginning of
    * calculation for a given signalId
    */
  virtual void Start (uint32_t signalId);

  /**
    * \brief Collect SpectrumValue and duration of signal for a given signalId
    *
    * Passed values are collected in m_sumValues and m_totDuration maps.
    */
  virtual void EvaluateChunk (const SpectrumValue& sinr, Time duration, uint32_t signalId);

  /**
    * \brief Finish calculation and inform interested objects about calculated value 
    * for a given signalId
    * During this function all callbacks from list are executed
    * to inform interested object about calculated value. This
    * function is called at the end of calculation.
    */
  virtual void End (uint32_t signalId);

private:
  std::map<uint32_t, Ptr<SpectrumValue> > m_sumValuesMap;
  std::map<uint32_t, Time > m_totDurationMap;

  std::vector<LteChunkProcessorMultipleCallback> m_LteChunkProcessorMultipleCallbacks;
};


/**
 * A sink to be plugged to the callback of LteChunkProcessorMultiple allowing
 * to save and later retrieve the latest reported value 
 * 
 */
class LteSpectrumValueCatcherMultiple
{
public:

  /** 
   * function to be plugged to LteChunkProcessorMultiple::AddCallback ()
   * 
   * \param value 
   */
  void ReportValue (const SpectrumValue& value);

  /** 
   * 
   * 
   * \return the latest value reported by the LteChunkProcessorMultiple
   */
  Ptr<SpectrumValue> GetValue ();
  
private:
  Ptr<SpectrumValue> m_value;
};

} // namespace ns3



#endif /* LTE_CHUNK_PROCESSOR_MULTIPLE_H */
