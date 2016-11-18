/**
 * \file components/gpp/phy/Dvbt1RSEncoder/Dvbt1RSEncoderComponent.cpp
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
 * Implementation of the Dvbt1RSEncoder component.
 */

#include "Dvbt1RSEncoderComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1RSEncoderComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1RSEncoderComponent::Dvbt1RSEncoderComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1rsencoder",                 // component type
                "A DVB-T1 R-S encoder component", // description
                "Giuseppe Baruffa",               // author
                "0.1")                            // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,tsOffset_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1RSEncoderComponent::~Dvbt1RSEncoderComponent()
{
  destroy();
}

/// Register the encoder ports with the IRIS system
///
/// This component has one input that accepts bytes and one output that
/// provides encoded bytes.
void Dvbt1RSEncoderComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1RSEncoderComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1RSEncoderComponent::initialize()
{
  setup();
}

// logarithm table
int Dvbt1RSEncoderComponent::index_[256] =
{
	255, 0, 1, 25, 2, 50, 26, 198, 3, 223, 51, 238, 27, 104, 199, 75,
	4, 100, 224, 14, 52, 141, 239, 129, 28, 193, 105, 248, 200, 8, 76, 113,
	5, 138, 101, 47, 225, 36, 15, 33, 53, 147, 142, 218, 240, 18, 130, 69,
	29, 181, 194, 125, 106, 39, 249, 185, 201, 154, 9, 120, 77, 228, 114, 166,
	6, 191, 139, 98, 102, 221, 48, 253, 226, 152, 37, 179, 16, 145, 34, 136,
	54, 208, 148, 206, 143, 150, 219, 189, 241, 210, 19, 92, 131, 56, 70, 64,
	30, 66, 182, 163, 195, 72, 126, 110, 107, 58, 40, 84, 250, 133, 186, 61,
	202, 94, 155, 159, 10, 21, 121, 43, 78, 212, 229, 172, 115, 243, 167, 87,
	7, 112, 192, 247, 140, 128, 99, 13, 103, 74, 222, 237, 49, 197, 254, 24,
	227, 165, 153, 119, 38, 184, 180, 124, 17, 68, 146, 217, 35, 32, 137, 46,
	55, 63, 209, 91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190, 97,
	242, 86, 211, 171, 20, 42, 93, 158, 132, 60, 57, 83, 71, 109, 65, 162,
	31, 45, 67, 216, 183, 123, 164, 118, 196, 23, 73, 236, 127, 12, 111, 246,
	108, 161, 59, 82, 41, 157, 85, 170, 251, 96, 134, 177, 187, 204, 62, 90,
	203, 89, 95, 176, 156, 169, 160, 81, 11, 245, 22, 235, 122, 117, 44, 215,
	79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168, 80, 88, 175
};

// power table
int Dvbt1RSEncoderComponent::alpha_[256] =
{
	1, 2, 4, 8, 16, 32, 64, 128, 29, 58, 116, 232, 205, 135, 19, 38,
	76, 152, 45, 90, 180, 117, 234, 201, 143, 3, 6, 12, 24, 48, 96, 192,
	157, 39, 78, 156, 37, 74, 148, 53, 106, 212, 181, 119, 238, 193, 159, 35,
	70, 140, 5, 10, 20, 40, 80, 160, 93, 186, 105, 210, 185, 111, 222, 161,
	95, 190, 97, 194, 153, 47, 94, 188, 101, 202, 137, 15, 30, 60, 120, 240,
	253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91, 182, 113, 226,
	217, 175, 67, 134, 17, 34, 68, 136, 13, 26, 52, 104, 208, 189, 103, 206,
	129, 31, 62, 124, 248, 237, 199, 147, 59, 118, 236, 197, 151, 51, 102, 204,
	133, 23, 46, 92, 184, 109, 218, 169, 79, 158, 33, 66, 132, 21, 42, 84,
	168, 77, 154, 41, 82, 164, 85, 170, 73, 146, 57, 114, 228, 213, 183, 115,
	230, 209, 191, 99, 198, 145, 63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
	227, 219, 171, 75, 150, 49, 98, 196, 149, 55, 110, 220, 165, 87, 174, 65,
	130, 25, 50, 100, 200, 141, 7, 14, 28, 56, 112, 224, 221, 167, 83, 166,
	81, 162, 89, 178, 121, 242, 249, 239, 195, 155, 43, 86, 172, 69, 138, 9,
	18, 36, 72, 144, 61, 122, 244, 245, 247, 243, 251, 235, 203, 139, 11, 22,
	44, 88, 176, 125, 250, 233, 207, 131, 27, 54, 108, 216, 173, 71, 142, 0
};

// generator polynomial
int Dvbt1RSEncoderComponent::gg_[17] =
{
  120, 225, 194, 182, 169, 147, 191, 91, 3, 76, 161, 102, 109, 107, 104, 120, 0
};

/// Encodes a single data packet
///
/// Provides, at the output, a systematic encoded codeword where the first 188
/// bytes are the message, and the last 16 bytes are the parity.
int Dvbt1RSEncoderComponent::packetEncode(unsigned char *data, unsigned char *bb)
{
	T1_CLEAR(bb,T1_NN-T1_KK);
	for(int i = T1_KK - 1; i >= 0; i--)
	{
		int feedback = index_[data[i] ^ bb[T1_NN_KK - 1]]; // feedback term
		if(feedback != T1_A0)
		{
		  // feedback term is non-zero
			for(int j = T1_NN_KK - 1; j > 0; j--)
				if(gg_[j] != T1_A0)
					bb[j] = bb[j - 1] ^ alpha_[modnn(gg_[j] + feedback)];
				else
					bb[j] = bb[j - 1];
			bb[0] = alpha_[modnn(gg_[0] + feedback)]; // terminal connection
		}
		else
		{
		  // feedback term is zero
			for(int j = T1_NN_KK - 1; j > 0; j--)
				bb[j] = bb[j - 1];
			bb[0] = 0;
		}
	}
	return 0;
}

/// Main processing method
void Dvbt1RSEncoderComponent::process()
{
  // request input
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);
  
  // calculate sizes
  int insize = in ? (int) in->data.size() : 0;
  int numpacks = (insize + tsOffset_) / TS_PACKET_SIZE;
  int outsize = numpacks * RS_PACKET_SIZE;

  // request output
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << insize + tsOffset_ << "(" << insize << "+" << tsOffset_ << ")/" << outsize;

  // fill the messagewords
  for(ByteVecIt init = in->data.begin(), outit = out->data.begin(); init < in->data.end(); init++)
  {
  
    // copy in reverse order
    rsCodeWord_[TS_PACKET_SIZE - 1 - tsOffset_] = *init;
    
    // trigger encoding
    if(++tsOffset_ == TS_PACKET_SIZE)
    {
      int status = packetEncode(rsCodeWord_, rsCodeWord_ + T1_KK);
			if (status)
				LOG(LERROR) << "Problem encoding a R-S word";

      // copy information part
      for(int b = 0; b < TS_PACKET_SIZE; b++, outit++)
        *outit = rsCodeWord_[TS_PACKET_SIZE - 1 - b];
			
			// copy parity part
			for(int b = 0; b < T1_NN_KK; b++, outit++)
				*outit = rsCodeWord_[T1_NN - 1 - b];
				
      // reset TS pointer
      tsOffset_ = 0;
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
void Dvbt1RSEncoderComponent::parameterHasChanged(std::string name)
{
  if(name == "???")
  {
    destroy();
    setup();
  }
}

/// Set up offsets and clean variables
void Dvbt1RSEncoderComponent::setup()
{
  // clean
  memset(rsCodeWord_, 0, T1_NN);
  tsOffset_ = 0;
}

/// Destroy the component
void Dvbt1RSEncoderComponent::destroy()
{
}

} // namespace phy
} // namespace iris
