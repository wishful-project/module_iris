/**
 * \file components/gpp/phy/Dvbt1ConvInterleaver/Dvbt1ConvInterleaverComponent.h
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
 * The Dvbt1ConvInterleaver component.
 */

#ifndef PHY_DVBT1CONVINTERLEAVERCOMPONENT_H_
#define PHY_DVBT1CONVINTERLEAVERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A DVB-T1 convolutional interleaver component.
 *
 * Dvbt1ConvInterleaverComponent is the third block composing the DVB-T transmission chain.
 * The purpose of this interleaver, placed between the R-S encoder and the
 * convolutional encoder, is most useful at decoding time. In fact, the
 * corresponding deinterleaver has the task to shuffle apart consecutive bursts
 * of errors coming out from the Viterbi decoder, so that the R-S error correcting
 * capability (up to 8 bytes in a codeword of 204 bytes) is not exceeded.
 * The convolutional interleaving process is based on the Forney approach, which
 * is compatible with the Ramsey type III approach, with  a depth of \f$I = 12\f$.
 * Each cell in every interleaving delay path is composed by \f$M_I=17\f$ bytes.
 * The interleaved data bytes are composed of error protected packets and are
 * delimited by inverted or non-inverted MPEG-2 sync bytes (204 bytes periodicity).
 *
 * \image html convinterleaver.png DVB-T convolutional interleaver.
 * \image latex convinterleaver.png DVB-T convolutional interleaver.
 *
 * Please note that the convolutional interleaver does not operate strictly as
 * a row-column block interleaver, since it keeps memory of older bytes in the
 * current block, and does not emit all the bytes in the current block. For a
 * different implementation of this block operation, please refer also to the
 * testing section implemented in MATLAB. In that case, the operation of this
 * interleaver is performed using a block row-column interleaver that is
 * _slant_ after data load and before data dump.
 *
 * There is only one parameter that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 * * Forney, G. D., _Burst-Correcting Codes for the Classic Bursty Channel_,
 *   IEEE Transactions on Communications, vol. COM-19, October 1971, pp. 772-781.
 * * Ramsey, J. L., _Realization of Optimum Interleavers_, IEEE Transactions
 *   on Information Theory, IT-16 (3), May 1970, pp. 338-345.
 */
class Dvbt1ConvInterleaverComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  Dvbt1ConvInterleaverComponent(std::string name);
  ~Dvbt1ConvInterleaverComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  
  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  
  int b_[12];                 ///< Interleaving ststus
  uint8_t I0_[1];             ///< First interleaving register, not used 
  uint8_t I1_[17];            ///< Second interleaving register
  uint8_t I2_[2*17];          ///< Third interleaving register
  uint8_t I3_[3*17];          ///< Fourth interleaving register
  uint8_t I4_[4*17];          ///< Fifth interleaving register
  uint8_t I5_[5*17];          ///< Sixth interleaving register
  uint8_t I6_[6*17];          ///< Seventh interleaving register
  uint8_t I7_[7*17];          ///< Eighth interleaving register
  uint8_t I8_[8*17];          ///< Ninth interleaving register
  uint8_t I9_[9*17];          ///< Tenth interleaving register
  uint8_t I10_[10*17];        ///< Eleventh interleaving register
  uint8_t I11_[11*17];        ///< Twelfth interleaving register
  int rsOffset_;              ///< Input offset
			
  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1CONVINTERLEAVERCOMPONENT_H_
