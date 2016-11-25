/**
 * \file components/gpp/phy/Dvbt1Interpolator/Dvbt1InterpolatorComponent.cpp
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
 * Implementation of the Dvbt1Interpolator component.
 */

#include "Dvbt1InterpolatorComponent.h"

#include <cmath>
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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1InterpolatorComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1InterpolatorComponent::Dvbt1InterpolatorComponent(std::string name)
  : PhyComponent(name,                               // component name
                "dvbt1interpolator",                 // component type
                "A DVB-T1 OFDM interpolator component", // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);

  registerParameter(
    "insamplerate", "Input sampling rate (use 0 for 9142857)",
    "0.0", true, inSampleRate_x, Interval<double>(0.0,15000000.0));

  registerParameter(
    "outsamplerate", "Output sampling rate (use 0 for 9142857)",
    "0.0", true, outSampleRate_x, Interval<double>(0.0,15000000.0));

  registerParameter(
    "responsefile", "Text file with the interpolating impulse response",
    "", true, responseFile_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1InterpolatorComponent::~Dvbt1InterpolatorComponent()
{
  destroy();
}

/// Register the mapper ports with the IRIS system
///
/// This component has one input that accept complex float values and one
/// output that provides complex float values.
void Dvbt1InterpolatorComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< Cplx >::identifier);
  registerOutputPort("output1", TypeInfo< Cplx >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide complex values.
void Dvbt1InterpolatorComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< Cplx >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1InterpolatorComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1InterpolatorComponent::process()
{
  // request input
  DataSet< Cplx > *in = NULL;
  getInputDataSet("input1", in);
  
  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int numbufs = (insize + inOffset_) / tiInsize_;
  int outsize = tiOutsize_ * numbufs;
    
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // request output
  DataSet< Cplx >* out = NULL;
  getOutputDataSet("output1", out, outsize);
  
  // copy
  for(CplxVecIt init = in->data.begin(), outit = out->data.begin(),
    inRegEff_ = inReg_.begin() + T1_RESAMPLE_ORDER + 1; init < in->data.end();
    init++)
  {
    // copy
    inRegEff_[inOffset_++] = *init;
    
    // do the trick
    if(inOffset_ == inLength_)
    {
      // reset
      inOffset_ = 0;
      
		  // fractional filter
		  for(int j = 0; j < tiOutsize_; j++, outit++)
		  {
			  // current base point
			  int currbp = tiBasepointIndex_[j];

			  // interpolate
			  Cplx temp(0,0);
			  for(int k = 0; k < T1_RESAMPLE_ORDER + 1; k++)
			  {
				  temp.real(temp.real() + inRegEff_[currbp - k].real() * tiHI_[k * tiOutsize_ + j]);
				  temp.imag(temp.imag() + inRegEff_[currbp - k].imag() * tiHI_[k * tiOutsize_ + j]);
			  }
			  *outit = temp;
		  }
		  
		  // copy last values at the beginning
		  copy(inReg_.end() - (T1_RESAMPLE_ORDER + 1), inReg_.end(), inReg_.begin());
    }
  }
                    
  //Copy the timestamp and sample rate for the DataSets
  out->timeStamp = in->timeStamp;
  out->sampleRate = in->sampleRate; // not sure about this: it should change! 
  
  // release input and output
  releaseOutputDataSet("output1", out);
  releaseInputDataSet("input1", in);
}

/// Actions taken when the parameters change
/// 
/// This block has several significant parameters
void Dvbt1InterpolatorComponent::parameterHasChanged(std::string name)
{
  if(name == "insamplerate" || name == "outsamplerate") 
  {
    destroy();
    setup();
  }
}

/// find a rational approximation of a real value
///
/// @param x Input value to be approximated as ratio of integers
/// @param N maximum value for the integer at the denominator
/// @param num the integer at the numerator
/// @param den the integer at the denominator
/// @return 0 for no  errors
int Dvbt1InterpolatorComponent::find_rational_approximation(int *num, int *den, double x, int N)
{
	int a = 0, b = 1;
	int c = 1, d = 0;
	while (b <= N && d <= N) {
		double mediant = (double) (a + c) / (double) (b + d);
		if (x == mediant) {
			if (b + d <= N) {
				*num = a + c;
				*den = b + d;
				return 0;
			} else if (d > b) {
				*num = c;
				*den = d;
				return 0;
			} else {
				*num = a;
				*den = b;
				return 0;
			}
		} else if (x > mediant) {
			a += c;
			b += d;
		} else {
			c += a;
			d += b;
		}
	}

	if (b > N) {
		*num = c;
		*den = d;
	} else {
		*num = a;
		*den = b;
	}
			
	return 0;
}

/// size correctly the interpolation buffers
///
/// @param input_samples Size, in samples, of the input buffer
/// @return Size, in samples, of the output buffer
int Dvbt1InterpolatorComponent::time_buffer_size(int input_samples)
{
	int output_samples = 0;
	int status = 0;

	// find the best rational approximation
	tiOutsize_ = 0;
	tiInsize_ = 0;
	status = find_rational_approximation(&tiOutsize_, &tiInsize_, outSampleRate_x / inSampleRate_x, 2000);
	if (status)
		LOG(LERROR) << "Could not find a rational approximation for " << outSampleRate_x << "/" << inSampleRate_x << "=" << outSampleRate_x / inSampleRate_x;

	LOG(LINFO) << "Original sampling rate: " << inSampleRate_x << " sps";
	LOG(LINFO) << "Effective sampling rate (x" << tiOutsize_ << "/" << tiInsize_ << "): " << inSampleRate_x * (double) tiOutsize_ / (double) tiInsize_ << " sps";

	return output_samples;
}

/// Calculate a Blackman-windowed sinc
///
/// @param n_order order of the calculated window
/// @param T time extension of the window
/// @param dt sampling time
/// @param order preferred order of the window
/// @return array containing the window taps, please remember to free when this
///         is not needed anymore
double *Dvbt1InterpolatorComponent::blackman_sinc(int *n_order, double T, double dt, int order)
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

/// sin(x)/x function
///
/// @param x Input value
/// @return The sinc of the input
double Dvbt1InterpolatorComponent::sinc(double x)
{
	return x == 0.0 ? 1.0 : (sin(x) / x);
}

/// interpolate a base response
///
/// @param h array of the taps of the base impulse response
/// @param n number of taps of the base impulse response
/// @param dt sampling time of the base impulse response
/// @param t time at which to interpolate the base response
/// @return the value of the base response linearly interpolated at the requested
///         time
double Dvbt1InterpolatorComponent::interp_response(double *h, int n, double dt, double t)
{
	if (t < 0.0)
		return 0.0;
	else if (t >= n * dt)
		return 0.0;
	else {
		int n0 = (int) floor(t / dt);
		double h0 = h[n0];
		double h1 = n0 == (n - 1) ? 0.0 : h[n0 + 1];
		return h0 + ((h1 - h0) / dt) * (t - n0 * dt);
	}
}

/// Set up all our index vectors and containers.
void Dvbt1InterpolatorComponent::setup()
{
  // calculate factors
  if(inSampleRate_x == 0)
    inSampleRate_x = 64.0e6/7.0;
  if(outSampleRate_x == 0)
    outSampleRate_x = 64.0e6/7.0;
  time_buffer_size(0);
  
  // clear
  inOffset_ = 0;
  inLength_ = tiInsize_;
  inReg_.resize(inLength_ + T1_RESAMPLE_ORDER + 1);
  
  // interpolator basepoint
  tiBasepointIndex_.resize(tiOutsize_);
	for(int i = 0; i < tiOutsize_; i++)
		tiBasepointIndex_[i] = (int) floor(inSampleRate_x * ((double) i /
		  outSampleRate_x));

  // interpolator response
  tiHI_.resize(tiOutsize_ * (T1_RESAMPLE_ORDER + 1));
	double dtbase = (1 / inSampleRate_x) / 100.0;
  int nbase = 0;
	double *hbase = blackman_sinc(&nbase, 1 / inSampleRate_x, dtbase,
	  T1_RESAMPLE_ORDER);
	for(int i = 0; i < T1_RESAMPLE_ORDER + 1; i++)
  {
		for(int j = 0; j < tiOutsize_; j++)
    {
			tiHI_[i * tiOutsize_ + j] =
        (float) interp_response(hbase, nbase, dtbase,
        ((double) j / outSampleRate_x) -
        ((double) (tiBasepointIndex_[j] - i) / inSampleRate_x));
    }
	}
	
	// dump to file
	if(!responseFile_x.empty())
  {
		FILE *ffp = fopen(responseFile_x.c_str(), "wt");
		for(int i = 0; i < nbase; i++)
			fprintf(ffp, "%.10f\n", hbase[i]);
		fclose(ffp);
	}
  free(hbase);

}

/// Destroy the component
void Dvbt1InterpolatorComponent::destroy()
{
}

} // namespace phy
} // namespace iris
