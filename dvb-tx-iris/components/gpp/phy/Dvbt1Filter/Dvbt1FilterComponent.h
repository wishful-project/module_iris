/**
 * \file components/gpp/phy/Dvbt1Filter/Dvbt1FilterComponent.h
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
 * The Dvbt1Filter component.
 */

#ifndef PHY_DVBT1FILTERCOMPONENT_H_
#define PHY_DVBT1FILTERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

namespace iris
{
namespace phy
{

/** A DVB-T1 filter component.
 *
 * Dvbt1FilterComponent is the second optional block composing the DVB-T
 * transmission chain. It is required only if the spectrum emission mask (SEM)
 * has to be obeyed directly at the BB level and cannot be modified operating
 * on the RF emitted signal. This filter also helps to reduce the IF images
 * resulting from the interpolation process, if the DAC sampling rate is not
 * directly compatible with the DVB-T sampling rate.
 *
 * This block implements a Kaiser-designed FIR lowpass filter, whose number of
 * taps is decided by the attenuation and transition bandwith values. Please note
 * that setting high values of attenuation or a steep transition bandwidth could
 * result in a high number of taps, and the filter could not be able to operate
 * in real time.
 *
 * This block accepts in input complex float values and
 * generates in output complex float values.
 *
 * There are parameters several that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *            information for the interested developer.
 * * _samplerate_: by default set to "0", a placeholder for 64e6/7 Hz. This
 *                 represents the sampling rate of the DAC signal and,
 *                 consequently, the whole bandwidth over which the filter may
 *                 operate.
 * * _stopband_: by default set to "4000000.0", it represents the frequency at
 *               which the specified attenuation is achieved. This frequency is
 *               given relatively to the centre frequency of the RF emitted signal.
 *               The transition bandwidth of the filter ends at this frequency,
 *               and it begins right after the last active OFDM carrier, which
 *               happens to be at 3.805 MHz for an 8K system.
 * * _attenuation_: by default set to "35.0", it is the attenuation (in dB) of
 *                  the filter at the specified stop frequency.
 * * _coeffsfile_: by default set to "", which means not enabled, this is the 
 *                 name of a text file where the impulse response of the filter
 *                 is saved, line after line.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)

 */
class Dvbt1FilterComponent
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

  /// A vector of float
  typedef std::vector<float>    FloatVec;
  
  /// An iterator for a vector of float
  typedef FloatVec::iterator    FloatVecIt;
  
  /// A vector of integers
  typedef std::vector<int>      IntVec;
  
  /// An iterator for a vector of integers
  typedef IntVec::iterator      IntVecIt;

  Dvbt1FilterComponent(std::string name);
  ~Dvbt1FilterComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;             ///< Debug flag (default = false)
  double sampleRate_x;      ///< Sampling rate (default = 0) 
  double stopBand_x;        ///< Filter stop-band (default = 4000000)
  double sBAttenuation_x;   ///< Filter stop-band attenuation (default = 35)
  std::string coeffsFile_x; ///< Text file with impulse response (default = none)

  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame
  bool symmetric_;            // filter symmetry
  int filterLength_;          // filter length (order + 1)
  FloatVec coeffp_;           // coefficients
  CplxVec work_;              // work array

  int kaiser_design(int *order, double *beta, double ripple, double width);
  int filter_design(FloatVec &h, int order, double fc);
  double kaiser_window(int n, int order, double beta);
  double sinc(double x);
  double factorial(int n);
  double bessel_I0(double x);

  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }
};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1FILTERCOMPONENT_H_
