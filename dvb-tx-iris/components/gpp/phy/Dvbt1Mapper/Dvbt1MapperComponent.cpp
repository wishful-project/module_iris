/**
 * \file components/gpp/phy/Dvbt1Mapper/Dvbt1MapperComponent.cpp
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
 * Implementation of the Dvbt1Mapper component.
 */

#include "Dvbt1MapperComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1MapperComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1MapperComponent::Dvbt1MapperComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1mapper",                    // component type
                "A DVB-T1 mapper component",      // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
    
  int qamarr[] = {4,16,64};
  registerParameter(
    "qammapping", "QAM constellation mapping",
    "16", true, qamMapping_x, list<int>(begin(qamarr),end(qamarr)));
    
  int harr[] = {0,1,2,4};
  registerParameter(
    "hyerarchymode", "Hyerarchical mode (0 = NH)",
    "0", true, hyerarchyMode_x, list<int>(begin(harr),end(harr)));
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1MapperComponent::~Dvbt1MapperComponent()
{
  destroy();
}

/// Register the mapper ports with the IRIS system
///
/// This component has one input that accept symbols (some bits per byte) and one
/// output that provides complex symbols (in floats).
void Dvbt1MapperComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< Cplx >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide complex values.
void Dvbt1MapperComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< Cplx >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1MapperComponent::initialize()
{
  setup();
}

/// Main processing method
void Dvbt1MapperComponent::process()
{
  // request input
  DataSet< uint8_t > *in = NULL;
  getInputDataSet("input1", in);

  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int outsize = insize;
  
  // request output
  DataSet< Cplx >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize << "/" << outsize;
    
  // assign
  CplxVecIt outit = out->data.begin();
  for(ByteVecIt init = in->data.begin(); init < in->data.end(); init++, outit++)
  {
    *outit = constel_[*init];
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
/// This block has two significant parameters
void Dvbt1MapperComponent::parameterHasChanged(std::string name)
{
  if(name == "qammapping" || name == "hyerarchymode")
  {
    destroy();
    setup();
  }
}

/// Set up all our constellations
void Dvbt1MapperComponent::setup()
{
  // nonuniformity value
	float alpha = hyerarchyMode_x == 0 ? 1 : (float) ceil((double) (1 << hyerarchyMode_x) / 2.0);
	
	// constellation array
  switch(qamMapping_x)
  {
    case 4:
      constel_.push_back(Cplx(1, 1));
      constel_.push_back(Cplx(1, -1));
      constel_.push_back(Cplx(-1, 1));
      constel_.push_back(Cplx(-1, -1));
      break;
    case 16:
      constel_.push_back(Cplx(3, 3));
      constel_.push_back(Cplx(3, 1));
      constel_.push_back(Cplx(1, 3));
      constel_.push_back(Cplx(1, 1));
      constel_.push_back(Cplx(3, -3));
      constel_.push_back(Cplx(3, -1));
      constel_.push_back(Cplx(1, -3));
      constel_.push_back(Cplx(1, -1));
      constel_.push_back(Cplx(-3, 3));
      constel_.push_back(Cplx(-3, 1));
      constel_.push_back(Cplx(-1, 3));
      constel_.push_back(Cplx(-1, 1));
      constel_.push_back(Cplx(-3, -3));
      constel_.push_back(Cplx(-3, -1));
      constel_.push_back(Cplx(-1, -3));
      constel_.push_back(Cplx(-1, -1));
      break;
    case 64:
      constel_.push_back(Cplx(7, 7));
      constel_.push_back(Cplx(7, 5));
      constel_.push_back(Cplx(5, 7));
      constel_.push_back(Cplx(5, 5));
      constel_.push_back(Cplx(7, 1));
      constel_.push_back(Cplx(7, 3));
      constel_.push_back(Cplx(5, 1));
      constel_.push_back(Cplx(5, 3));
      constel_.push_back(Cplx(1, 7));
      constel_.push_back(Cplx(1, 5));
      constel_.push_back(Cplx(3, 7));
      constel_.push_back(Cplx(3, 5));
      constel_.push_back(Cplx(1, 1));
      constel_.push_back(Cplx(1, 3));
      constel_.push_back(Cplx(3, 1));
      constel_.push_back(Cplx(3, 3));
      constel_.push_back(Cplx(7, -7));
      constel_.push_back(Cplx(7, -5));
      constel_.push_back(Cplx(5, -7));
      constel_.push_back(Cplx(5, -5));
      constel_.push_back(Cplx(7, -1));
      constel_.push_back(Cplx(7, -3));
      constel_.push_back(Cplx(5, -1));
      constel_.push_back(Cplx(5, -3));
      constel_.push_back(Cplx(1, -7));
      constel_.push_back(Cplx(1, -5));
      constel_.push_back(Cplx(3, -7));
      constel_.push_back(Cplx(3, -5));
      constel_.push_back(Cplx(1, -1));
      constel_.push_back(Cplx(1, -3));
      constel_.push_back(Cplx(3, -1));
      constel_.push_back(Cplx(3, -3));
      constel_.push_back(Cplx(-7, 7));
      constel_.push_back(Cplx(-7, 5));
      constel_.push_back(Cplx(-5, 7));
      constel_.push_back(Cplx(-5, 5));
      constel_.push_back(Cplx(-7, 1));
      constel_.push_back(Cplx(-7, 3));
      constel_.push_back(Cplx(-5, 1));
      constel_.push_back(Cplx(-5, 3));
      constel_.push_back(Cplx(-1, 7));
      constel_.push_back(Cplx(-1, 5));
      constel_.push_back(Cplx(-3, 7));
      constel_.push_back(Cplx(-3, 5));
      constel_.push_back(Cplx(-1, 1));
      constel_.push_back(Cplx(-1, 3));
      constel_.push_back(Cplx(-3, 1));
      constel_.push_back(Cplx(-3, 3));
      constel_.push_back(Cplx(-7, -7));
      constel_.push_back(Cplx(-7, -5));
      constel_.push_back(Cplx(-5, -7));
      constel_.push_back(Cplx(-5, -5));
      constel_.push_back(Cplx(-7, -1));
      constel_.push_back(Cplx(-7, -3));
      constel_.push_back(Cplx(-5, -1));
      constel_.push_back(Cplx(-5, -3));
      constel_.push_back(Cplx(-1, -7));
      constel_.push_back(Cplx(-1, -5));
      constel_.push_back(Cplx(-3, -7));
      constel_.push_back(Cplx(-3, -5));
      constel_.push_back(Cplx(-1, -1));
      constel_.push_back(Cplx(-1, -3));
      constel_.push_back(Cplx(-3, -1));
      constel_.push_back(Cplx(-3, -3));
      break;
  }
  
  // add alpha and find energy
  float energy = 0;
  for(int m = 0; m < constel_.size(); m++)
  {
    constel_[m].real(constel_[m].real() + (alpha - 1) * (constel_[m].real() >= 0 ? 1 : -1));
    constel_[m].imag(constel_[m].imag() + (alpha - 1) * (constel_[m].imag() >= 0 ? 1 : -1));
    energy += constel_[m].real() * constel_[m].real() + constel_[m].imag() * constel_[m].imag();
  }
	energy = sqrtf(energy / constel_.size());

  // normalize to have unit energy
  for(int m = 0; m < constel_.size(); m++)
  {
		constel_[m].real(constel_[m].real() / energy);
		constel_[m].imag(constel_[m].imag() / energy);
	}
}

/// Destroy the component
void Dvbt1MapperComponent::destroy()
{
}

} // namespace phy
} // namespace iris
