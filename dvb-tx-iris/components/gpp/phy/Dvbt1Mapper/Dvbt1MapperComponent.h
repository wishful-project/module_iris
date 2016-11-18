/**
 * \file components/gpp/phy/Dvbt1Mapper/Dvbt1MapperComponent.h
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
 * The Dvbt1Mapper component.
 */

#ifndef PHY_DVBT1MAPPERCOMPONENT_H_
#define PHY_DVBT1MAPPERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A DVB-T1 mapper component.
 *
 * Dvbt1MapperComponent is the eighth block composing the DVB-T transmission chain.
 * The mapper uses the QAM constellations mandated in the standard, to transform the
 * data symbols into complex numbers that can be eventually delivered over I&Q
 * analog waveforms. The constellations are Gray-encoded, that is, adjacent points
 * in the complex plane only differ in one bit among their represented symbols
 * (indeed, this is only partially true, as there can also be a difference of more
 * bits, but at larger distances).
 *
 * \image html mapping.png DVB-T 16-QAM constellation.
 * \image latex mapping.png DVB-T 16-QAM constellation.
 *
 * The constellation points are statically written in the source files.
 *
 * This blocks accepts in input elements in uint8_t (\f$\nu\f$-bit symbols) and
 * generates in output complex values (complex float).
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
class Dvbt1MapperComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  /// A complex type
  typedef std::complex<float>   Cplx;

  /// A vector of complex
  typedef std::vector<Cplx>     CplxVec;

  // An iterator for a vector of complex
  typedef CplxVec::iterator     CplxVecIt;

  Dvbt1MapperComponent(std::string name);
  ~Dvbt1MapperComponent();
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
  
  CplxVec constel_;           ///< actual constellation
     			
  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1MAPPERCOMPONENT_H_
