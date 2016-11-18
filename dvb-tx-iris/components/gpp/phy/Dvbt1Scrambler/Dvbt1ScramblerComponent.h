/**
 * \file components/gpp/phy/Dvbt1Scrambler/Dvbt1ScramblerComponent.h
 * \version 0.1
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2016 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * The Dvbt1Scrambler component.
 */

#ifndef PHY_DVBT1SCRAMBLERCOMPONENT_H_
#define PHY_DVBT1SCRAMBLERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace iris
{
namespace phy
{

/** A DVB-T energy dispersal component
 *
 * Dvbt1ScramblerComponent is the first block composing the DVB-T transmission chain.
 * This block takes an MPEG-2 Transport Stream (TS) of data
 * bytes in uint8_t format and outputs a scrambled stream of uint8_t data. Per the
 * DVB-T standard, the PRBS generator polynomial is \f$1+X^{14}+X^{15}\f$. The scrambler
 * operates on a group of eight TS packets: each packet is 188-byte long and 
 * begins with the SYNC byte, 0x47. The PRBS register is loaded with the sequence
 * "100101010000000" and is shift-enabled after the eighth bit, thus the ninth
 * bit is the first to be scrambled. The other 7 SYNC bytes in the group are then
 * bitwise-inverted to 0xB8, so as to provide a viable means for recovering scrambling
 * synchrony at the receiver. The process is then repeated for the following 
 * groups of eight packets. The full period of the scrambling sequence is thus of
 * 188 * 8 - 1 = 1503 bytes.
 *
 * \image html scrambler.png DVB-T energy dispersal.
 * \image latex scrambler.png DVB-T energy dispersal.
 *
 * There are two parameters that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 * * _reportinterval_: by default set to "0", which means it is disabled. If a number
 *                   greater than zero is used, then it will be the number of seconds
 *                   between which the block reports the computed processing speed.
 *                   This can be useful to benchmark on-the-fly the processing
 *                   speed of a complete DVB-T modulator graph that uses this
 *                   block as source: if the graph is free-running, i.e., not
 *                   terminated into an USRP block or similar, it will provide
 *                   the maximum TS bitrate that the CPU is capable to process.
 *                   Differently, if terminated into an USRP, this can be used
 *                   to verify if the expected bitrate value (for that particular
 *                   combination of DVB-T modulation and coding parameters) is
 *                   honored.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 */
class Dvbt1ScramblerComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  Dvbt1ScramblerComponent(std::string name);
  ~Dvbt1ScramblerComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  double reportInterval_x;    ///< Reporting interval in seconds (default = 0)
  
  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  
  int scramblerOffset_;       ///< Current scrambling offset
  boost::posix_time::ptime start_; ///< Timestamp used for frame error rate reports
  uint64_t doneBytes_;        ///< currently processed bytes

  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

  static uint8_t scramblerPrbs_[1504]; ///< Scrambling PRBS bytes

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1SCRAMBLERCOMPONENT_H_
