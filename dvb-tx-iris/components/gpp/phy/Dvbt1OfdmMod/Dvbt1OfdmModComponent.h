/**
 * \file components/gpp/phy/Dvbt1OfdmMod/Dvbt1OfdmModComponent.h
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
 * The Dvbt1OfdmMod component.
 */

#ifndef PHY_DVBT1OFDMMODCOMPONENT_H_
#define PHY_DVBT1OFDMMODCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include "irisapi/PhyComponent.h"
#include "fftw3.h"

#define T1_BLOCKS_PER_FRAME	68
#define T1_FRAMES_PER_SUPERFRAME	4
#define T1_RESAMPLE_ORDER 4

namespace iris
{
namespace phy 
{

/** A DVB-T1 OFDM modulator component.
 *
 * Dvbt1OfdmModComponent is the tenth block composing the DVB-T transmission chain.
 * The OFDM block takes the modulated QAM cells, assembled in frames together with
 * the pilot and TPS cells, and converts them from a _virtual_ frequency domain
 * sequence to a time domain signal, which can be transmitted on a channel.
 *
 * Not all the carriers are modulated, but some of them are left untouched for
 * purposes of guard bandwidth implementation (_virtual_ carriers). Due to the
 * peculiar way frequencies are structured in the sampled frequency domain,
 * the central part of the spectrum is left to the virtual carriers, whereas
 * the outer portions are occupied by the active carriers.
 *
 * \image html ofdm1.png OFDM carriers arrangement.
 * \image latex ofdm1.png OFDM carriers arrangement.
 *
 * The conversion between the frequency and time domains can be done in several
 * ways, either by using a bank of quadrature modulators or an inverse Discrete
 * Frequency Transform algorithm. In our case, we use an inverse Fast Fourier
 * Transform algorithm, and the signal generated starting from the active cells
 * \f$\Psi_k\f$ can be written as
 * \f[
 * x\left[n\right] = \frac{1}{N_{\rm FFT}} \sum_{k=0}^{N_{\rm FFT}-1}
 * { \rho_k \Lambda_k \Psi_k e^{j \frac{2 \pi}{N_{\rm FFT}} k n}}
 * \, , \quad 
 * n=-L,-\left(L-1\right),\ldots,1,0,1,2,\ldots,\left(N_{\rm FFT}-1\right) \, ,
 * \f]
 * where \f$L\f$ is the cyclic prefix size,
 * \f$\rho_k\f$ is a frequency amplitude linear precorrection term and 
 * \f$\Lambda_k\f$ is a power-loading factor: the purpose of these terms
 * will be clarified below.
 *
 * This block accepts in input complex float values and
 * generates in output complex float values.
 *
 * There are several parameters that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 * * _ofdmmode_: by default set to "2048", this is used to select one of the three
 *               possible OFDM modes. The admitted values are "2048", "4096",
 *               "8192", respectively for 2K, 4K (DVB-H, unused), and 8K.
 * * _deltamode_: by default set to "32", this is used to select one of the four
 *                possible cyclic prefix lengths. The admitted values are "32", 
 *                "16", "8", and "4", which are directly derived from the
 *                denominator of the cyclic prefix fraction (1/32, 1/16, 1/8, 1/4).
 * * _outpower_: by default set to "10", this parameter represents the scaling
 *               factor used for rescaling the IFFT output into the wanted range.
 *               In particular, this parameter is a percentage. A percentage of
 *               100 means that the output signal real and imaginary parts have
 *               an amplitude distribution that concentrates the values into a
 *               interval between -1 and 1 with the 99.7% of probability. Since the
 *               OFDM signal is Gaussian, this means that the \f$\pm 3 \sigma\f$
 *               interval of amplitudes falls in the span \f$\left[-1,+1\right]\f$.
 *               When the digital signal is mapped onto analog values by the 
 *               USRP DAC, for example, the valid range is that enclosed in the
 *               \f$\left[-1,+1\right]\f$ interval, all other values will be clipped.
 * * _dacsamplerate_: by default set to "0", a placeholder for 64e6/7 Hz. This
 *                    represents
 *                    the sampling rate adopted by the DAC for emitting
 *                    the BB analog signal. It is used internally to precorrect,
 *                    linearly, with a multiplicative factor \f$\rho_k\f$, the 
 *                    amplitude of the OFDM carriers that will be
 *                    distorted by the Dvbt1Interpolator block. The type of distortion 
 *                    is decided by the algorithm adopted internally by
 *                    the interpolator block. **Please note that
 *                    if you are not using the Dvbt1Interpolator block, then you
 *                    need to leave this parameter at 0**.
 * * _powerfile_: by default empty, this is the name of a text file that can be
 *                read, at periodic intervals, to generate a powerloading
 *                configuration for the OFDM carriers. This file contains, line
 *                by line, the value of power correction, expressed in dB, for
 *                each one of the OFDM carriers. For instance, for 8K OFDM, the
 *                file is composed by 8192 lines. A value of 0 means that the
 *                power of the carrier is left untouched, a positive valiue means
 *                that there will be a power increase, a negative value will
 *                result into a power decrease. The positioning of the carrier
 *                indices starts from the first, lowest frequency carrier up to 
 *                the last, highest frequency carrier.
 * * _powerinterval_: by default it is set to "1". This is the number of seconds
 *                    among consecutive reads of the power loading file.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 */
class Dvbt1OfdmModComponent
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

  // An iterator for a vector complex of
  typedef CplxVec::iterator     CplxVecIt;

  /// A vector of float
  typedef std::vector<float>    FloatVec;
  
  /// An iterator for a vector of float
  typedef FloatVec::iterator    FloatVecIt;
  
  /// A vector of integers
  typedef std::vector<int>      IntVec;
  
  /// An iterator for a vector of typedef
  typedef IntVec::iterator      IntVecIt;

  Dvbt1OfdmModComponent(std::string name);
  ~Dvbt1OfdmModComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  int ofdmMode_x;             ///< OFDM mode (default = 2048)
  int deltaMode_x;            ///< Cyclic prefix ratio (default = 32)
  float outPower_x;           ///< Output power indicator (default = 10)
  double dacSampleRate_x;     ///< Sampling rate used by the DAC
  std::string powerFile_x;    ///< Text file with power loading (default = none)
  double powerInterval_x;     ///< Power update interval in seconds (default = 0)

  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame

  int nFft_;           // FFT useful length
  int nDelta_;         // prefix length
  int nBlock_;         // block length (useful + prefix)
  int inOffset_;       // input register offset
  CplxVec inReg_;      // input register
  CplxVec fftReg_;     // FFT register
  int nMax_;           // data carriers
  int kMax_;           // active carriers
  int tpsNum_;         // TPS carriers
  int nBit_;           // number of bits in FFT addressing
  float multFactor_;   // multiplicative factor for power definition
  FloatVec _precorrFactor_;   // precorrection vector
  FloatVecIt precorrFactor_;  // offset'ed precorrection
  FloatVec _ampliFactor_;    // powerloading vector
  FloatVecIt ampliFactor_;   // offset'ed powerloading vector
  fftwf_plan fft_;           ///< Our FFT object pointer.
  Cplx* fftBins_;            ///< Allocated using fftwf_malloc (SIMD aligned)
			
  void powerProcedure_();
  boost::thread *powerThread_;
  bool runPower_;      // true when the powerloading thread is running
  
  double sinc(double x);
  double frequency_response_modulus(double *h, int n, double dt, double f);
  double *blackman_sinc(int *n_order, double T, double dt, int order);

  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1OFDMMODCOMPONENT_H_
