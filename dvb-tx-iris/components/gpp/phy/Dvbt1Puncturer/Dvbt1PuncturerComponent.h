/**
 * \file components/gpp/phy/Dvbt1Puncturer/Dvbt1PuncturerComponent.h
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
 * The Dvbt1Puncturer component.
 */

#ifndef PHY_DVBT1PUNCTURERCOMPONENT_H_
#define PHY_DVBT1PUNCTURERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A DVB-T1 puncturer component.
 *
 * Dvbt1PuncturerComponent is the fifth block composing the DVB-T transmission chain.
 * The purpose of the puncturer is that of achieving variable coding rate keeping
 * fixed the properties and complexity of the main _mother_ convolutional code,
 * which sticks at a rate of \f$k/n=1/2\f$. By properly removing convolutional
 * encoded bits before transmission, one can still expect to take profit of the
 * error correction capabilities of the convolutional decoder (Viterbi algorithm)
 * at the receiving side, although having the possibility to change the overall
 * coding rate to one of the values \f$r_c = 1/2, 2/3, 3/4, 5/6, 7/8\f$.
 * The block operates by translating the puncturing matrices, that are given in the
 * standard, into a periodic subsampling structure (a sort of nonequispaced bit
 * decimation). Thus, a group of input bits (at the input periodicity) are read into
 * a register, which is dumped into another shorter register, which in turn is
 * read out (at the output periodicity).
 *
 * \image html puncturer.png DVB-T pucturer.
 * \image latex puncturer.png DVB-T puncturer.
 *
 * This block accepts in input elements in uint8_t (bits) and generates
 * in output bits (uint8_t).
 *
 * There are two parameters that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 * * _coderate_: by default set to "34", this is used to select one of the five
 *               possible coding rates. The admitted values are "12", "23", "34",
 *               "56", and "78", which are easily recognizable as the real coding
 *               ratioes written without the separating slash.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 * * S. Li, D. J. Costello, _Error Control Coding, Second Edition_, Prentice-Hall,
 *   Inc. Upper Saddle River, NJ, USA, 2004
 */
class Dvbt1PuncturerComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  Dvbt1PuncturerComponent(std::string name);
  ~Dvbt1PuncturerComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  int codeRate_x;             ///< stream channel coding rate (default = 34) 
 
  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  
  int punOffset_;                  ///< Puncturing offset
  uint8_t punRegister_[14];        ///< Puncturing register (statically set to
                                   ///  the maximum exected size)
  int punPeriodIn_, punPeriodOut_; ///< Input and output puncturing periods
			
  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1PUNCTURERCOMPONENT_H_
