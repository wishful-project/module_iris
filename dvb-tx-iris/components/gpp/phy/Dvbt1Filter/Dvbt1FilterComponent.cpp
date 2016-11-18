/**
 * \file components/gpp/phy/Dvbt1Filter/Dvbt1FilterComponent.cpp
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
 * Implementation of the Dvbt1Filter component.
 */

#include "Dvbt1FilterComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1FilterComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1FilterComponent::Dvbt1FilterComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1filter",                    // component type
                "A DVB-T1 filter component",      // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
    
  registerParameter(
    "samplerate", "Sampling rate (use 0 for 9142857)",
    "0.0", true, sampleRate_x, Interval<double>(0.0,15000000.0));

  registerParameter(
    "stopband", "Stop-band of the filter, in Hz relative to the "
    "centre frequency", "4000000.0", true, stopBand_x,
    Interval<double>(2000000.0,10000000.0));

  registerParameter(
    "attenuation", "Attenuation in the stop-band, in dB: 0 disables "
    "filtering (35 is the value tested at Electrosys)", "35.0", true,
    sBAttenuation_x, Interval<double>(0.0,90.0));
    
  registerParameter(
    "coeffsfile", "Text file with the filter impulse response",
    "", true, coeffsFile_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1FilterComponent::~Dvbt1FilterComponent()
{
  destroy();
}

/// Register the mapper ports with the IRIS system
///
/// This component has one input that accept complex float values and one
/// output that provides complex float values.
void Dvbt1FilterComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< Cplx >::identifier);
  registerOutputPort("output1", TypeInfo< Cplx >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide complex values.
void Dvbt1FilterComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< Cplx >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1FilterComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1FilterComponent::process()
{
  // request input
  DataSet< Cplx > *in = NULL;
  getInputDataSet("input1", in);
  
  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = insize;
    
  // request output and pre-fill with zeroes
  DataSet< Cplx >* out = NULL;
  getOutputDataSet("output1", out, insize);
  fill(out->data.begin(), out->data.end(), Cplx(0,0));
  
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // copy head
  CplxVecIt workit = work_.begin() + filterLength_ - 1;
  copy(in->data.begin(), in->data.begin() + filterLength_ - 1, workit);
  
  // filter!
  CplxVecIt outit = out->data.begin();                     
	if(symmetric_)
	{
		// symmetric filter
		for(int n = 0; n < filterLength_ - 1; n++, outit++)
		{
			CplxVecIt init = workit + n;
			CplxVecIt inlastit = workit + n - filterLength_ + 1;
      FloatVecIt coeffit = coeffp_.begin();
			for(; init > inlastit; init--, inlastit++, coeffit++)
			{
				outit->real(outit->real() + *coeffit * (init->real() + inlastit->real()));
				outit->imag(outit->imag() + *coeffit * (init->imag() + inlastit->imag()));
			}
			outit->real(outit->real() + *coeffit * init->real());
			outit->imag(outit->imag() + *coeffit * init->imag());
		}
		for(int n = filterLength_ - 1; n < insize; n++, outit++)
		{
			CplxVecIt init = in->data.begin() + n;
			CplxVecIt inlastit = in->data.begin() + n - filterLength_ + 1;
      FloatVecIt coeffit = coeffp_.begin();
			for(; init > inlastit; init--, inlastit++, coeffit++)
			{
				outit->real(outit->real() + *coeffit * (init->real() + inlastit->real()));
				outit->imag(outit->imag() + *coeffit * (init->imag() + inlastit->imag()));
			}
			outit->real(outit->real() + *coeffit * init->real());
			outit->imag(outit->imag() + *coeffit * init->imag());
		}
	} else {
		// asymmetric filter - double work
		for(int n = 0; n < filterLength_ - 1; n++, outit++)
		{
			CplxVecIt init = workit + n;
			for(FloatVecIt coeffit = coeffp_.begin(); coeffit < coeffp_.end(); 
			  coeffit++, init--)
			{
				outit->real(outit->real() + *coeffit * init->real());
				outit->imag(outit->imag() + *coeffit * init->imag());
			}
		}
		for(int n = filterLength_ - 1; n < insize; n++, outit++)
		{
			CplxVecIt init = in->data.begin() + n;
			for(FloatVecIt coeffit = coeffp_.begin(); coeffit < coeffp_.end(); 
			  coeffit++, init--)
			{
				outit->real(outit->real() + *coeffit * init->real());
				outit->imag(outit->imag() + *coeffit * init->imag());
			}
		}
	}
	   
	// copy tail in previous
	copy(in->data.end() - (filterLength_ - 1), in->data.end(), work_.begin());
	
  // Copy the timestamp and sample rate for the DataSets
  out->timeStamp = in->timeStamp;
  out->sampleRate = in->sampleRate;
  
  // release input and output
  releaseOutputDataSet("output1", out);
  releaseInputDataSet("input1", in);
}

/// Actions taken when the parameters change
/// 
/// This block has several significant parameters
void Dvbt1FilterComponent::parameterHasChanged(std::string name)
{
  if(name == "stopband" || name == "attenuation") 
  {
    destroy();
    setup();
  }
}

/// sin(x)/x function
///
/// @param x Input value
/// @return The sinc of the input
double Dvbt1FilterComponent::sinc(double x)
{
	return x == 0.0 ? 1.0 : (sin(x) / x);
}

/// factorial function
///
/// @param n the integer on which to apply the factorial
/// @return the factorial of n
double Dvbt1FilterComponent::factorial(int n)
{
	double fact = 1.0;
	int i = 0;
	for (i = 1; i <= n; i++)
		fact *= (double) i;
	return fact;
}

/// Zeroth Order Modified Bessel Function
///
/// @param x The input value
/// @return The function evaluated on x
double Dvbt1FilterComponent::bessel_I0(double x)
{
	double I0 = 1.0;
	int i = 0;
	for (i = 1; i <= 20; i++) {
		I0 += pow(x / 2.0, (double) (2 * i)) / pow(factorial(i), 2.0);
	}
	return I0;
}

/// Find Kaiser parameters
///
/// Implementation inspired from
/// http://www.labbookpages.co.uk/audio/firWindowing.html
///
/// @param ripple Ripple of the filter (linear)
/// @param width Bandwidth normalized to sampling frequency
/// @param beta The \f$\beta\f$ of the Kaiser window
/// @param order The order of the filter (number of taps minus one)
/// @return 0 in case of success
int Dvbt1FilterComponent::kaiser_design(int *order, double *beta, double ripple,
  double width)
{
	double A = -20.0 * log(ripple) / log(10.0);
	double tw = 2.0 * M_PI * width;

	if (A > 21.0)
		*order = (int) ceil((A - 7.95) / (2.285 * tw));
	else
		*order = (int) ceil(5.79 / tw);

	if (A <= 21.0)
		*beta = 0.0;
	else if (21.0 < A && A <= 50.0)
		*beta = 0.5842 * pow(A - 21.0, 0.4) + 0.07886 * (A - 21.0);
	else
		*beta = 0.1102 * (A - 8.7);

	return 0;
}

/// Design a Kaiser-windowed low-pass filter
///
/// Implementation inspired from
/// http://www.labbookpages.co.uk/audio/firWindowing.html
///
/// @param fc The cut frequency is normalized to the sampling frequency
/// @param order Order of the filter (as calculated by kaiser_design)
/// @param h The array of filter taps
/// @return 0 in case of success
int Dvbt1FilterComponent::filter_design(FloatVec &h, int order, double fc)
{
	h.resize(order + 1);
	for (int i = 0; i <= order; i++) {
		h[i] = (float) (2.0 * fc * sinc(2.0 * M_PI * fc * ((double) i
		  - (double) order / 2.0)));
	}

	return 0;
}

/// Find Kaiser window coefficients
///
/// Inspired from http://www.labbookpages.co.uk/audio/firWindowing.html
///
/// @param n The lag at which to evaluate the Kaiser function
/// @param beta The \f$\beta\f$ of the Kaiser window
/// @param order The order of the filter (number of taps minus one)
/// @return The amplitude of the filter tap
double Dvbt1FilterComponent::kaiser_window(int n, int order, double beta)
{
	return bessel_I0(beta * sqrt(1.0 - pow(((double) (2 * n) / (double) order)
	  - 1.0, 2.0))) / bessel_I0(beta);
}

/// Change this to suit your needs
#define MAX_FILTER_LENGTH	127

/// unused
#define MAX_FILTER_LENGTH_2	50001

/// Set up all our index vectors and containers
void Dvbt1FilterComponent::setup()
{
  // replace the DVB-T sample rate with its real value
  if(sampleRate_x == 0)
    sampleRate_x = 64.0e6/7.0;
    
  // clear
  symmetric_ = true;
  filterLength_ = 1;
  coeffp_.resize(1);
  coeffp_[0] = 1;
  
  // test section, leave disabled
  if(false)
  {
    symmetric_ = true;
    filterLength_ = 123;
    coeffp_.resize(filterLength_);
    for(int i = 0; i < filterLength_; i++)
      coeffp_[i] = ((double) i / 4.0)/filterLength_;
  }
     
	// design the transmission filter if requested
	if(true && sBAttenuation_x)
	{
		// checks (you can try to modify the limits, but long filters could result
		if(stopBand_x < 0.515 * (64.0e6/7.0) * (1705.0 / 2048.0))
			LOG(LERROR) << "The selected stopband is too next to the passband: "
			  << stopBand_x;
		if(stopBand_x > 0.485 * sampleRate_x)
			LOG(LERROR) << "The selected stopband is too next to the sampling band: "
			  << sampleRate_x;
		if(sBAttenuation_x > 40)
			LOG(LERROR) << "A maximum attenuation of 40 dB can be specified";
		if (sBAttenuation_x < 5)
			LOG(LERROR) << "A minimum attenuation of 5 dB can be specified";

		// the transition width is between the last carrier edge and the stopband
		double tw = stopBand_x - 0.5 * (64.0e6/7.0) * (1705.0 / 2048.0);

		// the cutoff frequency is at the last carrier edge plus half transition width
		double fc = 0.501 * (64.0e6/7.0) * (1705.0 / 2048.0) + tw / 2;

		// normalize to sample frequency
		tw /= sampleRate_x;
		fc /= sampleRate_x;

		// the ripple
		double ripple = pow(10.0, - sBAttenuation_x / 20.0);

		// find kaiser parameters
		double beta = 0.0;
		int order = 0;
		int status = kaiser_design(&order, &beta, ripple, tw);
		if(status)
			LOG(LERROR) << "Could not design the Kaiser window";

		// ensure an integer-delay filter is designed (odd length)
		filterLength_ = (2 *((order + 1) / 2)) + 1;

		// check
		if(filterLength_ > MAX_FILTER_LENGTH)
			LOG(LERROR) << "The maximum filter length has been exceeded: relax the "
			  "filtering performance";

		// design base filter
		status = filter_design(coeffp_, filterLength_ - 1, fc);
		if(status)
			LOG(LERROR) << "Could not design the base filter";

		// windowed filter
		for(int m = 0; m < filterLength_; m++)
			coeffp_[m] *= (float) kaiser_window(m, filterLength_ - 1, beta);

		// dump filter coefficients to file
	  if(!coeffsFile_x.empty())
    {
		  FILE *fp = fopen(coeffsFile_x.c_str(), "wt");
		  if(fp)
		  {
        setlocale(LC_NUMERIC, "C");
			  for(int m = 0; m < filterLength_; m++)
				  fprintf(fp, "%.8f\n", coeffp_[m]);
			  fclose(fp);
		  }
		}

		// discover if the filter is symmetric or asymmetric
		// This isn't really needed, since the filter will always be symmetrical
		double maxtol = 1.0E-8, tol = 0.0;
		for (int m = 0; m < filterLength_ / 2; m++)
			tol += fabs(coeffp_[m] - coeffp_[filterLength_ - 1 - m]);
		if (tol < maxtol)
			symmetric_ = true;
		else
			symmetric_ = false;
	}

	// resume filter characteristics
	LOG(LINFO) << (symmetric_ ? "Symmetric" : "Asymmetric") << " filter, "
	  << filterLength_ << " taps";

  // working initial array
  work_.resize(filterLength_ - 1 + filterLength_ - 1);
}

/// Destroy the component
void Dvbt1FilterComponent::destroy()
{
}

} // namespace phy
} // namespace iris
