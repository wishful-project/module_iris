/**
 * \file components/gpp/phy/Dvbt1ConvEncoder/Dvbt1ConvEncoderComponent.cpp
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
 * Implementation of the Dvbt1ConvEncoder component.
 */

#include "Dvbt1ConvEncoderComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1ConvEncoderComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1ConvEncoderComponent::Dvbt1ConvEncoderComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1convencoder",                 // component type
                "A DVB-T1 convolutional encoder component", // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,status_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1ConvEncoderComponent::~Dvbt1ConvEncoderComponent()
{
  destroy();
}

/// Register the scrambler ports with the IRIS system
///
/// This component has one input that accepts bytes and one output that
/// provides convolutional encoded bits (one bit per byte).
void Dvbt1ConvEncoderComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1ConvEncoderComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1ConvEncoderComponent::initialize()
{
  setup();
}

/// This look-up tables contains pairs of convolutional encoder parity bit outputs
/// for all the possible configurations of states (64) and inputs (2)
unsigned char Dvbt1ConvEncoderComponent::parity_[256] = {
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0,
};

/// First polynomial (bit-reversed)
#define	g1		0x4f

/// Second polynomial (bit-reversed)
#define	g2		0x6d

/// Main processing method
void Dvbt1ConvEncoderComponent::process()
{
  // request input
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);

  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = 2 * 8 * insize;

  // request output - double size
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // iterate over all input bytes
  for(ByteVecIt init = in->data.begin(), outit = out->data.begin(); init < in->data.end(); init++)
  {
    // iterate over all the bits of the byte
    for(int j = 7; j >= 0; j--)
    {
      status_ = (status_ << 1) | ((*init >> j) & 0x01); // new status
      *outit++ = parity_[status_ & g1]; // first parity bit
      *outit++ = parity_[status_ & g2]; // second parity bit
    }
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
void Dvbt1ConvEncoderComponent::parameterHasChanged(std::string name)
{
  if(name == "???")
  {
    destroy();
    setup();
  }
}

/// Clean variables
void Dvbt1ConvEncoderComponent::setup()
{
  status_ = 0;
}

/// Destroy the component
void Dvbt1ConvEncoderComponent::destroy()
{
}

} // namespace phy
} // namespace iris
