/**
 * \file components/gpp/phy/Dvbt1ConvInterleaver/Dvbt1ConvInterleaverComponent.cpp
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
 * Implementation of the Dvbt1ConvInterleaver component.
 */

#include "Dvbt1ConvInterleaverComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1ConvInterleaverComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1ConvInterleaverComponent::Dvbt1ConvInterleaverComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1convinterleaver",                 // component type
                "A DVB-T1 convolutional interleaver component", // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,rsOffset_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1ConvInterleaverComponent::~Dvbt1ConvInterleaverComponent()
{
  destroy();
}

/// Register the interleaver ports with the IRIS system
///
/// This component has one input that accepts bytes and one output that
/// provides interleaved bytes.
void Dvbt1ConvInterleaverComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1ConvInterleaverComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1ConvInterleaverComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1ConvInterleaverComponent::process()
{
  // request input
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);

  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = insize;

  // request output
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // process data with a humongous Duff's device!
  // For more info on what a Duff is, check the Wikipedia page
  // https://en.wikipedia.org/wiki/Duff's_device
  ByteVecIt init = in->data.begin();
  ByteVecIt outit = out->data.begin();
  switch(rsOffset_)
  {
    // in the following, we load a bunch of bytes in the generic path
    // and advance the relevant pointer
    case 0:
    case 12:
    do
    {
      // first, direct path
      *outit++ = *init++;
      rsOffset_ = 1;
		  if(init == in->data.end())
		    break;

    case 1:
      // second path
      *outit++ = I1_[b_[1]];
      I1_[b_[1]] = *init++;
      if(++(b_[1]) == 17 * 1)
        b_[1] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;
    
    case 2:      
      // third path
      *outit++ = I2_[b_[2]];
      I2_[b_[2]] = *init++;
      if(++(b_[2]) == 17 * 2)
        b_[2] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 3:
      // fourth path
      *outit++ = I3_[b_[3]];
      I3_[b_[3]] = *init++;
      if(++(b_[3]) == 17 * 3)
        b_[3] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 4:
      // fifth path
      *outit++ = I4_[b_[4]];
      I4_[b_[4]] = *init++;
      if(++(b_[4]) == 17 * 4)
        b_[4] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;
    
    case 5:
      // sixth path
      *outit++ = I5_[b_[5]];
      I5_[b_[5]] = *init++;
      if(++(b_[5]) == 17 * 5)
        b_[5] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 6:
		  // seventh path
      *outit++ = I6_[b_[6]];
      I6_[b_[6]] = *init++;
      if(++(b_[6]) == 17 * 6)
        b_[6] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 7:
      // eighth path
      *outit++ = I7_[b_[7]];
      I7_[b_[7]] = *init++;
      if(++(b_[7]) == 17 * 7)
        b_[7] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 8:
      // ninth path
      *outit++ = I8_[b_[8]];
      I8_[b_[8]] = *init++;
      if(++(b_[8]) == 17 * 8)
        b_[8] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 9:
      // tenth path
      *outit++ = I9_[b_[9]];
      I9_[b_[9]] = *init++;
      if(++(b_[9]) == 17 * 9)
        b_[9] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 10:
      // eleventh path
      *outit++ = I10_[b_[10]];
      I10_[b_[10]] = *init++;
      if(++(b_[10]) == 17 * 10)
        b_[10] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;

    case 11:
      // twelfth and final path
      *outit++ = I11_[b_[11]];
      I11_[b_[11]] = *init++;
      if(++(b_[11]) == 17 * 11)
        b_[11] = 0;
      rsOffset_++;
      if(init == in->data.end())
        break;
        
    } while(true);
  }
  
  //Copy the timestamp and sample rate for the DataSets
  out->timeStamp = in->timeStamp;
  out->sampleRate = in->sampleRate;

  // release input and output
  releaseInputDataSet("input1", in);
  releaseOutputDataSet("output1", out);
}

/// Actions taken when the parameters change
/// 
/// This block has no significant parameters
void Dvbt1ConvInterleaverComponent::parameterHasChanged(std::string name)
{
  if(name == "???")
  {
    destroy();
    setup();
  }
}

/// Set up offsets and clean interleaver registers
///
/// Please note that filling the registers with zeroes, as we do below, generates
/// peaky transients in the final waveform right after the system start-up and
/// up to the moment when all registers are filled by real data bytes.
/// In order to avoid this, we should fill this with a random sequence of bytes,
/// instead of zeroes. 
void Dvbt1ConvInterleaverComponent::setup()
{
  // clean registers
  memset(b_, 0, sizeof(b_));
  memset(I0_, 0, sizeof(I0_));
  memset(I1_, 0, sizeof(I1_));
  memset(I2_, 0, sizeof(I2_));
  memset(I3_, 0, sizeof(I3_));
  memset(I4_, 0, sizeof(I4_));
  memset(I5_, 0, sizeof(I5_));
  memset(I6_, 0, sizeof(I6_));
  memset(I7_, 0, sizeof(I7_));
  memset(I8_, 0, sizeof(I8_));
  memset(I9_, 0, sizeof(I9_));
  memset(I10_, 0, sizeof(I10_));
  memset(I11_, 0, sizeof(I11_));

  // reset the offset
  rsOffset_ = 0;
}

/// Destroy the component
void Dvbt1ConvInterleaverComponent::destroy()
{
}

} // namespace phy
} // namespace iris
