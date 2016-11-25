/**
 * \file components/gpp/phy/Dvbt1Puncturer/Dvbt1PuncturerComponent.cpp
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
 * Implementation of the Dvbt1Puncturer component.
 */

#include "Dvbt1PuncturerComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1PuncturerComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1PuncturerComponent::Dvbt1PuncturerComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1puncturer",                 // component type
                "A DVB-T1 puncturer component",   // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,punOffset_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);

  int codearr[] = {12,23,34,56,78};
  registerParameter(
    "coderate", "Channel coding rate",
    "34", true, codeRate_x, list<int>(begin(codearr),end(codearr)));
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1PuncturerComponent::~Dvbt1PuncturerComponent()
{
  destroy();
}

/// Register the puncturer ports with the IRIS system
///
/// This component has one input that accepts bits (one bit per byte) and one
/// output that provides punctured bits (one bit per byte).
void Dvbt1PuncturerComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1PuncturerComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1PuncturerComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1PuncturerComponent::process()
{
  // request input
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);

  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = ((insize + punOffset_) / punPeriodIn_) * punPeriodOut_;

  // request output
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize + punOffset_ << "(" << insize << "+" << punOffset_ << ")/" << outsize;
    
  // iterate over input
  for(ByteVecIt init = in->data.begin(), outit = out->data.begin(); init < in->data.end(); init++)
  {
    // fill puncturing register
    punRegister_[punOffset_++] = *init;
    
    // trigger puncturing at the output
    if(punOffset_ == punPeriodIn_)
    {
      // reset offset
      punOffset_ = 0;
      
      // copy to output
      switch(codeRate_x)
      {
        // the puncturing matrices are hard-coded for all the five code rates
        case 12:
          *outit++ = punRegister_[0];
          *outit++ = punRegister_[1];
          break;
        case 23:
          *outit++ = punRegister_[0];
          *outit++ = punRegister_[1];
          *outit++ = punRegister_[3];
          break;
        case 34:
          *outit++ = punRegister_[0];
          *outit++ = punRegister_[1];
          *outit++ = punRegister_[3];
          *outit++ = punRegister_[4];
          break;
        case 56:
          *outit++ = punRegister_[0];
          *outit++ = punRegister_[1];
          *outit++ = punRegister_[3];
          *outit++ = punRegister_[4];
          *outit++ = punRegister_[7];
          *outit++ = punRegister_[8];
          break;
        case 78:
          *outit++ = punRegister_[0];
          *outit++ = punRegister_[1];
          *outit++ = punRegister_[3];
          *outit++ = punRegister_[5];
          *outit++ = punRegister_[7];
          *outit++ = punRegister_[8];
          *outit++ = punRegister_[11];
          *outit++ = punRegister_[12];
          break;
        default:
          LOG(LERROR) << "Invalid puncturing rate: " << codeRate_x;    
      }
    }
  }
  
  // Copy the timestamp and sample rate for the DataSets
  out->timeStamp = in->timeStamp;
  out->sampleRate = in->sampleRate;

  // release input and output
  releaseInputDataSet("input1", in);
  releaseOutputDataSet("output1", out);
}

/// Actions taken when the parameters change
/// 
/// This block has one significant parameters
void Dvbt1PuncturerComponent::parameterHasChanged(std::string name)
{
  if(name == "coderate")
  {
    destroy();
    setup();
  }
}

/// Set up all puncturing basic sizes, reset offsets, clean registers
void Dvbt1PuncturerComponent::setup()
{
  switch(codeRate_x)
  {
    case 12:
      punPeriodIn_ = 2;
      punPeriodOut_ = 2;
      break;
    case 23:
      punPeriodIn_ = 4;
      punPeriodOut_ = 3;
      break;
    case 34:
      punPeriodIn_ = 6;
      punPeriodOut_ = 4;
      break;
    case 56:
      punPeriodIn_ = 10;
      punPeriodOut_ = 6;
      break;
    case 78:
      punPeriodIn_ = 14;
      punPeriodOut_ = 8;
      break;
    default:
      LOG(LERROR) << "Invalid puncturing rate: " << codeRate_x;
  }
  punOffset_ = 0;
  memset(punRegister_, 0, sizeof(punRegister_));
}

/// Destroy the component
void Dvbt1PuncturerComponent::destroy()
{
}

} // namespace phy
} // namespace iris
