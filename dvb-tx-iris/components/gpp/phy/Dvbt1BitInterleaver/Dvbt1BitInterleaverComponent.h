/**
 * \file components/gpp/phy/Dvbt1BitInterleaver/Dvbt1BitInterleaverComponent.h
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
 * The Dvbt1BitInterleaver component.
 */

#ifndef PHY_DVBT1BITINTERLEAVERCOMPONENT_H_
#define PHY_DVBT1BITINTERLEAVERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A DVB-T1 bit interleaver component.
 *
 * Dvbt1BitInterleaverComponent is the sixth block composing the DVB-T transmission chain.
 * Its purpose, together with the symbol interleaver, is that of reordering the
 * channel encoded bits in order to convert the possible error bursts arising from
 * the communication on the physical channel (due to impulsive noise, multipath,
 * fading) into well-separated single-error events. This way, the channel decoders
 * at the RX side (Viterbi and Reed-Solomon decoder) are able to perform at their
 * best theoretical limit in white Gaussian noise (WGN) conditions.
 *
 * \image html bitinterleaver.png DVB-T bit interleaver.
 * \image latex bitinterleaver.png DVB-T bit interleaver.
 *
 * With reference to the figure above, the input demultiplexer routes the incoming
 * bits emitted by the puncturer towards one of the 6 bit interleaving RAMs.
 * Every RAM has a capacity of 126 bits. When all the RAMs are filled, the stored bits
 * are read out according to a particular cyclic address shift and composed into
 * \f$\nu\f$-bit symbols, where \f$\nu\f$ is the number of bits of the particular
 * M-QAM mapping adopted. Please note that only \f$\nu\f$ RAM interleavers are
 * adopted, thus the figure above refers to the 64-QAM case.
 * This block accepts in input elements in uint8_t (bits) and generates
 * in output \f$\nu\f$-bit symbols (uint8_t).
 *
 * There are three parameters that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 * * _qammapping_: by default set to "16", this is used to select one of the three
 *               possible QAM mappings. The admitted values are "4", "16", "64".
 * * _hyerarchymode_: by default set to "0", which means "not hyerarchical".
 *                    Hierarchical modes are used to transmit two different transport
 *                    streams, one with a high priority (HP) information and 
 *                    another one with a low priority (LP) information. The admitted
 *                    values are "0, "1", "2", "4". NOTE: hyerarchical modes 
 *                    are not implemented in the current release of
 *                    this modulator.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 */
class Dvbt1BitInterleaverComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  Dvbt1BitInterleaverComponent(std::string name);
  ~Dvbt1BitInterleaverComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  int qamMapping_x;           ///< QAM constellation mapping (default = 16)
  int hyerarchyMode_x;        ///< Hyerarchical mode (default = 0)
 
  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  
  int intOffset_[2];          ///< Interleaving offsets (HP & LP)
  int intLength_[2];          ///< Interleaving registers length (HP & LP)
  uint8_t *intRegister_[2];   ///< Interleaving registers (HP & LP)
  int nu_;                    ///< Bits per modulated symbol

  // interleaving addresses
  static int address_v2[252];
  static int address_v4[504];
  static int address_v6[756];
  			
  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }
};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1BITINTERLEAVERCOMPONENT_H_
