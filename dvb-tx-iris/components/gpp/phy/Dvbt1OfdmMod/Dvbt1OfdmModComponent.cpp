/**
 * \file components/gpp/phy/Dvbt1OfdmMod/Dvbt1OfdmModComponent.cpp
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
 * Implementation of the Dvbt1OfdmMod component.
 */

#include "Dvbt1OfdmModComponent.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace boost::lambda;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1OfdmModComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1OfdmModComponent::Dvbt1OfdmModComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1ofdmmod",                   // component type
                "A DVB-T1 OFDM modulator component", // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,fft_(NULL)
    ,fftBins_(NULL)
    ,powerThread_(NULL)
    ,runPower_(false)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
   
  int ofdmarr[] = {2048,4096,8192};
  registerParameter(
    "ofdmmode", "OFDM mode",
    "2048", true, ofdmMode_x, list<int>(begin(ofdmarr),end(ofdmarr)));

  int deltaarr[] = {32,16,8,4};
  registerParameter(
    "deltamode", "Cyclic prefix ratio",
    "32", true, deltaMode_x, list<int>(begin(deltaarr),end(deltaarr)));

  registerParameter(
    "outpower", "Output power, in percentage: note that a value of 100 will "
    "result in signal clipping only below -3 sigma and above +3 sigma",
    "10", true, outPower_x, Interval<float>(0,300));

  registerParameter(
    "dacsamplerate", "Sampling rate at the DAC (default = 0, means 64e6/7)",
    "0", true, dacSampleRate_x, Interval<double>(0,15000000));
    
  registerParameter(
    "powerfile", "Text file with the power loading profile (default = none)",
    "", true, powerFile_x);

  registerParameter(
    "powerinterval", "Power update interval in seconds (default = 1)",
    "0", true, powerInterval_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1OfdmModComponent::~Dvbt1OfdmModComponent()
{
  destroy();
}

/// Register the mapper ports with the IRIS system
///
/// This component has one input that accept complex float values and one
/// output that provides complex float values.
void Dvbt1OfdmModComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< Cplx >::identifier);
  registerOutputPort("output1", TypeInfo< Cplx >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide complex values.
void Dvbt1OfdmModComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< Cplx >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1OfdmModComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1OfdmModComponent::process()
{
  // request input
  DataSet< Cplx > *in = NULL;
  getInputDataSet("input1", in);

  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = nBlock_ * ((insize + inOffset_) / kMax_);
  
  // request output
  DataSet< Cplx >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // fill the input register
  CplxVecIt outit = out->data.begin();
  for(CplxVecIt init = in->data.begin(); init < in->data.end(); init++)
  {
    // copy datum
    inReg_[inOffset_++] = *init;

    // trigger IFFT
    if(inOffset_ == kMax_)
    {
	    int num_pos = kMax_ / 2 + 1;
	    int num_neg = num_pos - 1;
	    int neg_start = nFft_ - num_neg;

      // reset offset
      inOffset_ = 0;

      // copy positive frequencies
      for(int i = 0; i < num_pos; i++)
      {
        fftBins_[i] = inReg_[num_neg + i] * precorrFactor_[i] * ampliFactor_[i];
      }

      // copy negative frequencies
      for(int i = 0; i < num_neg; i++)
      {
        fftBins_[neg_start + i] = inReg_[i] * precorrFactor_[-num_neg + i] * ampliFactor_[-num_neg + i];
      }

      // set null frequencies
      for(int i = 0; i < nFft_ - kMax_; i++)
      {
        fftBins_[num_pos + i] = Cplx(0,0);
      }

      // call FFTW
      fftwf_execute(fft_);

      // apply multiplicative factor, for the power
      for(int i = 0; i < nFft_; i++)
      {
	      fftBins_[i].real(fftBins_[i].real() * multFactor_);
	      fftBins_[i].imag(fftBins_[i].imag() * multFactor_);
      }

      // copy to output
      CplxVecIt it = copy(&fftBins_[nFft_ - nDelta_], &fftBins_[nFft_], outit);
      copy(&fftBins_[0], &fftBins_[nFft_], it);

      outit += nBlock_;
    }
  }
                
  //set the timestamp and sample rate for the DataSets
  out->timeStamp = timeStamp_;
  out->sampleRate = sampleRate_;
  timeStamp_ += (double) outsize / sampleRate_;

  // release input and output
  releaseInputDataSet("input1", in);
  releaseOutputDataSet("output1", out);
}

/// Actions taken when the parameters change
/// 
/// This block has several significant parameters
void Dvbt1OfdmModComponent::parameterHasChanged(std::string name)
{
  if(name == "deltamode" || name == "ofdmmode" ||
      name == "outpower")
  {
    destroy();
    setup();
  }
}

/// sin(x)/x function
///
/// @param x Input value
/// @return The sinc of the input
double Dvbt1OfdmModComponent::sinc(double x)
{
	return x == 0.0 ? 1.0 : (sin(x) / x);
}

/// Calculate the frequency response modulus of an impulse response
///
/// @param h array of impulse response taps
/// @param n number of taps
/// @param dt sampling period of the impulse response
/// @param f frequency at which the modulus of the frequency response is calculated
/// @return the modulus of the frequency response at the indicated frequency
double Dvbt1OfdmModComponent::frequency_response_modulus(double *h, int n, double dt, double f)
{
	double H_re = 0.0, H_im = 0.0;
	double arg = 2.0 * M_PI * f * dt;
	int i;

  // just use plain old DFT, no FFT here
	for (i = 0; i < n; i++) {
		H_re += h[i] * cos(arg * i) * dt;
		H_im += h[i] * (-sin(arg * i)) * dt;
	}

	return sqrt(H_re * H_re + H_im * H_im);
}

/// Calculate a Blackman-windowed sinc
///
/// @param n_order order of the calculated window
/// @param T time extension of the window
/// @param dt sampling time
/// @param order preferred order of the window
/// @return array containing the window taps, please remember to free when this
///         is not needed anymore
double *Dvbt1OfdmModComponent::blackman_sinc(int *n_order, double T, double dt, int order)
{
	int n0 = (int) floor(T / dt);
	int i;
	double *h_order = NULL, w = 0.0;
	double a0 = 7938.0 / 18608.0, a1 = 9240.0 / 18608.0, a2 = 1430.0 / 18608.0;
	double accum = 0.0;
	*n_order = (order + 1) * n0;
	h_order = (double *) calloc(*n_order, sizeof(double));
	for (i = 0; i < *n_order; i++) {
		w = a0 - a1 * cos(2.0 * M_PI * i / (*n_order - 1)) + a2 * cos(4.0 * M_PI * i / (*n_order - 1));
		h_order[i] = w * sinc(M_PI * (i - *n_order / 2) * dt / T);
		accum += h_order[i] * dt;
	}
	/*for (i = 0; i < *n_order; i++)
		h_order[i] /= accum;*/

	return h_order;
}

/// Set up all needed constants
void Dvbt1OfdmModComponent::setup()
{
  if(dacSampleRate_x == 0)
    dacSampleRate_x = 64.0e6/7.0;
  sampleRate_ = 64.0e6/7;
  timeStamp_ = 0;
  // clean registers
  switch(ofdmMode_x)
  {
    case 2048:
      tpsNum_ = 17;
      nMax_ = 1512;
      kMax_ = 1705;
      nBit_ = 11;
      nFft_ = 2048;
      break;
    case 4096:
      tpsNum_ = 34;
      nMax_ = 3024;
      kMax_ = 3409;
      nBit_ = 12;
      nFft_ = 4096;
      break;
    case 8192:
      tpsNum_ = 68;
      nMax_ = 6048;
      kMax_ = 6817;
      nBit_ = 13;
      nFft_ = 8192;
      break;
  }
  nDelta_ = (int) floor((double) nFft_ / (double) deltaMode_x);
  nBlock_ = nFft_ + nDelta_;
  inOffset_ = 0;
  inReg_.resize(kMax_);
  fftReg_.resize(nFft_);

	int num_pos = kMax_ / 2 + 1;
	int num_neg = num_pos - 1;
	float temp = 0.0F;

  // multiplicative factor
	float power = (1.0F * (float) nMax_ /* data carriers */
		+ (16.0F / 9.0F) * (float) (kMax_ - nMax_ - tpsNum_) /* pilot carriers */
		+ 1.0F * (float) tpsNum_ /* tps carriers */)
		/ (float) nFft_; 
  multFactor_ = (float) sqrt((outPower_x / 100.0F) / (power * (float) nFft_)) /
    3.0F;

  // linear precorrection
	double dtbase = (1 / (64.0e6/7.0)) / 100.0;
  int nbase = 0;
	double *hbase = blackman_sinc(&nbase, 1 / (64.0e6/7.0), dtbase, T1_RESAMPLE_ORDER);
  _precorrFactor_.resize(nFft_);
  precorrFactor_ = _precorrFactor_.begin() + nFft_ / 2;
	for(int i = -num_neg; i < num_pos; i++)
  {
    //printf("dacSampleRate_x = %f\n", dacSampleRate_x);
		if(dacSampleRate_x == 64.0e6/7.0)
		{
			precorrFactor_[i] = 1.0F; // no precorrection
		}
		else
			precorrFactor_[i] = (float) (1.0 / frequency_response_modulus(hbase,
			  nbase, dtbase, (double) i * (64.0e6/7.0) / (double) nFft_));

		if (i == 0)
			temp = precorrFactor_[i];
	}
	
	// normalize
	for(int i = -num_neg; i < num_pos; i++)
		precorrFactor_[i] /= temp;
  free(hbase);

  // amplitude power loading factor
  _ampliFactor_.resize(nFft_);
  ampliFactor_ = _ampliFactor_.begin() + nFft_ / 2;
	for(int i = -num_neg; i < num_pos; i++)
    ampliFactor_[i] = 1.0F; // default powerloading
	if(!powerFile_x.empty())
  {
    // stop in case it's running
    if(powerThread_)
    {
      runPower_ = false;
      powerThread_->join();
      delete powerThread_;
    }
    
    // start thread
    runPower_ = true;
    powerThread_ = new boost::thread(boost::bind(&Dvbt1OfdmModComponent::powerProcedure_, this));
	}
	
  // Set up containers for FFTW
  fftBins_ = reinterpret_cast<Cplx*>(
      fftwf_malloc(sizeof(fftwf_complex) * ofdmMode_x));
  fill(&fftBins_[0], &fftBins_[ofdmMode_x], Cplx(0,0));
  fft_ = fftwf_plan_dft_1d(ofdmMode_x,
                           (fftwf_complex*)fftBins_,
                           (fftwf_complex*)fftBins_,
                           FFTW_BACKWARD,
                           FFTW_MEASURE);

}

#define WAKEUPINTERVALMS 200

/// Separate thread for power loading
void Dvbt1OfdmModComponent::powerProcedure_()
{
  int currentTick = 0, nextTick = currentTick;
  
  while(runPower_)
  {    
    if(currentTick == nextTick)
    {
      // advance ticks
      nextTick = currentTick + 1000 * powerInterval_x / WAKEUPINTERVALMS;
      
      // open the file
      std::ifstream myfile(powerFile_x.c_str());
      if(myfile)
      {
        // read it line after line
        std::string line;
        int l = 0;
        while (std::getline(myfile, line) && l < nFft_)
        {
          // parse the line and get the dB, then convert it to linear
          float val = 0.0f;
          std::istringstream istr(line);
          istr.imbue(std::locale("C"));
          istr >> val;
          _ampliFactor_[l++] = powf(10.0F, val / 20.0F);
        }
      }
      else
      {
        LOG(LINFO) << "Power loading file '" << powerFile_x << "' not found";
      }
    }

    // this is to give responsivity
    boost::this_thread::sleep(boost::posix_time::milliseconds(WAKEUPINTERVALMS));
    currentTick++;
  }
}

/// Destroy the component
void Dvbt1OfdmModComponent::destroy()
{
  if(fftBins_ != NULL)
    fftwf_free(fftBins_);
  if(fft_ != NULL)
    fftwf_destroy_plan(fft_);
    
  // stop thread
  runPower_ = false;
  if (powerThread_) {
    powerThread_->join();
    delete powerThread_; 
  }
}

} // namespace phy
} // namespace iris
