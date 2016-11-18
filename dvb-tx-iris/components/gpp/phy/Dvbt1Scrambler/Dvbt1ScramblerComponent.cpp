/**
 * \file components/gpp/phy/Dvbt1Scrambler/Dvbt1ScramblerComponent.cpp
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
 * Implementation of the Dvbt1Scrambler component.
 */

#include "Dvbt1ScramblerComponent.h"

#include <cmath>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

#include "irisapi/LibraryDefs.h"
#include "irisapi/Version.h"

using namespace std;
using namespace boost::lambda;
using namespace boost::posix_time;

namespace iris
{
namespace phy
{

// export library symbols
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1ScramblerComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1ScramblerComponent::Dvbt1ScramblerComponent(std::string name)
  : PhyComponent(name,                          // component name
                "dvbt1scrambler",               // component type
                "A DVB-T1 scrambler component", // description
                "Giuseppe Baruffa",             // author
                "0.1")                          // version
    ,sampleRate_(0)
    ,timeStamp_(0)
    ,scramblerOffset_(0)
{
  registerParameter(
    "debug", "Whether to output debug data",
    "false", true, debug_x);
    
  registerParameter(
    "reportinterval", "Report interval in seconds",
    "0", true, reportInterval_x);
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1ScramblerComponent::~Dvbt1ScramblerComponent()
{
  destroy();
}

/// Register the scrambler ports with the IRIS system
///
/// This component has one input that accepts TS bytes and one output that
/// provides scrambled bytes.
void Dvbt1ScramblerComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1ScramblerComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1ScramblerComponent::initialize()
{
  setup();
}

/// Scrambling sequence bytes
uint8_t Dvbt1ScramblerComponent::scramblerPrbs_[1504] = { 
  255,   3, 246,   8,  52,  48, 184, 163, 147, 201, 104, 183, 115, 179,  41, 170,
  245, 254,  60,   4, 136,  27,  48,  90, 161, 223, 196, 192, 154, 131,  95,  11,
  194,  56, 140, 147,  43, 106, 251, 126,  27,   4,  90,  25, 220,  84, 201, 250,
  180,  31, 184,  65, 145, 133, 101,  31,  94,  67, 197, 136, 157,  51,  78, 171,
  167, 249, 208,  20, 224, 122,  65,  29, 134,  77,  21, 174, 125, 229,  12,  94,
  41, 196, 244, 154,  59,  92, 155, 203,  88, 187, 211, 152, 233,  82, 119, 237,
  48, 110, 161, 103, 199,  80, 147, 227, 104,  75, 113, 187,  37, 154, 221,  94,
  207, 198, 160, 151, 195, 112, 139,  35,  58, 202, 158, 191,  71, 131, 145,   9,
  102,  55,  84, 179, 251, 168,  25, 240,  84,  33, 248, 196,  18, 152, 111,  81,
  99, 231,  72,  83, 177, 233, 164, 117, 217,  60, 214, 138, 247,  62,  50, 132,
  175,  27, 226,  88,  77, 209, 172, 229, 234,  92, 125, 201,  12, 182,  43, 180,
  249, 186,  21, 156, 125,  73,  15, 182,  33, 180, 197, 186,   0, 159,  77,  67,
  175, 137, 225,  52,  70, 185, 151, 149, 113, 127,  39,   2, 210,  14, 236,  38,
  104, 213, 114, 255,  46,   2, 228,  14,  88,  37, 208, 220, 226, 202,  78, 189,
  167, 141, 209,  44, 230, 234,  86, 125, 245,  12,  62,  40, 132, 243,  26,  42,
  92, 253, 202,  12, 188,  43, 136, 249,  50,  22, 172, 119, 233,  48, 118, 161,
  55, 198, 176, 151, 163, 113, 203,  36, 186, 219, 158, 217,  70, 215, 150, 241,
  118,  39,  52, 210, 186, 239, 158,  97,  69,  71, 159, 145,  65, 103, 135,  81,
  19, 230, 104,  85, 113, 255,  36,   2, 216,  14, 208,  38, 224, 214,  66, 245,
  142,  61,  36, 142, 219,  38, 218, 214, 222, 246, 198,  54, 148, 183, 123, 179,
  25, 170,  85, 253, 252,  12,   8,  40,  48, 240, 162,  35, 204, 200, 170, 179,
  255, 168,   1, 240,   4,  32,  24, 192,  82, 129, 239,   4,  98,  25,  76,  87,
  169, 241, 244,  36,  56, 216, 146, 211, 110, 235, 102, 123,  85,  27, 254,  88,
  5, 208,  28, 224,  74,  65, 189, 133,   0,  29,  46,  78, 229, 166,  93, 213,
  204, 252, 170,  11, 252,  56,   8, 144,  51,  96, 171,  67, 251, 136,  25,  48,
  86, 161, 247, 196,  48, 152, 163,  83, 203, 232, 184, 115, 145,  41, 102, 247,
  86,  51, 244, 168,  59, 240, 152,  35,  80, 203, 226, 184,  79, 145, 161, 101,
  199,  92, 147, 203, 104, 187, 115, 155,  41,  90, 247, 222,  48, 196, 162, 155,
  207,  88, 163, 211, 200, 232, 178, 115, 173,  41, 238, 244, 102,  57,  84, 151,
  251, 112,  27,  32,  90, 193, 222, 132, 199,  26, 146,  95, 109, 195, 108, 139,
  107,  59, 122, 155,  31,  90,  67, 221, 136, 205,  50, 174, 175, 231, 224,  80,
  65, 225, 132,  69,  25, 158,  85,  69, 255, 156,   1,  72,   7, 176,  17, 160,
  101, 193,  92, 135, 203,  16, 186,  99, 157,  73,  79, 183, 161, 177, 197, 164,
  157, 219,  76, 219, 170, 217, 254, 212,   6, 248,  22,  16, 116,  97,  57,  70,
  151, 151, 113, 115,  39,  42, 210, 254, 238,   6, 100,  21,  88, 127, 209,   0,
  230,   2,  84,  13,   0,  44,  16, 232,  98, 113,  77,  39, 174, 209, 230, 228,
  86,  89, 245, 212,  60, 248, 138,  19,  60, 106, 137, 127,  55,   2, 178,  15,
  172,  33, 232, 196, 114, 153,  47,  86, 227, 246,  72,  53, 176, 189, 163, 141,
  201,  44, 182, 235, 182, 121, 181,  21, 190, 125, 133,  13,  30,  46,  68, 229,
  154,  93,  93, 207, 204, 160, 171, 195, 248, 136,  19,  48, 106, 161, 127, 199,
  0, 146,   3, 108,  11, 104,  59, 112, 155,  35,  90, 203, 222, 184, 199, 146,
  145, 111, 103,  99,  83,  75, 235, 184, 121, 145,  21, 102, 127,  85,   3, 254,
  8,   4,  48,  24, 160,  83, 193, 232, 132, 115,  25,  42,  86, 253, 246,  12,
  52,  40, 184, 243, 146,  41, 108, 247, 106,  51, 124, 171,  11, 250,  56,  28,
  144,  75,  97, 187,  69, 155, 157,  89,  79, 215, 160, 241, 194,  36, 140, 219,
  42, 218, 254, 222,   6, 196,  22, 152, 119,  81,  51, 230, 168,  87, 241, 240,
  36,  32, 216, 194, 210, 142, 239,  38,  98, 213,  78, 255, 166,   1, 212,   4,
  0,  26,  16,  92,  97, 201,  68, 183, 155, 177,  89, 167, 213, 208, 252, 226,
  10,  76,  61, 168, 141, 243,  44,  42, 232, 254, 114,   5,  44,  30, 232,  70,
  113, 149,  37, 126, 223,   6, 194,  22, 140, 119,  41,  50, 246, 174,  55, 228,
  176,  91, 161, 217, 196, 212, 154, 251,  94,  27, 196,  88, 153, 211,  84, 235,
  250, 120,  29,  16,  78,  97, 165,  69, 223, 156, 193,  74, 135, 191,  17, 130,
  101,  13,  94,  47, 196, 224, 154,  67,  93, 139, 205,  56, 174, 147, 231, 104,
  83, 113, 235,  36, 122, 217,  30, 214,  70, 245, 150,  61, 116, 143,  59,  34,
  154, 207,  94, 163, 199, 200, 144, 179,  99, 171,  73, 251, 180,  25, 184,  85,
  145, 253, 100,  15,  88,  35, 208, 200, 226, 178,  79, 173, 161, 237, 196, 108,
  153, 107,  87, 123, 243,  24,  42,  80, 253, 226,  12,  76,  41, 168, 245, 242,
  60,  44, 136, 235,  50, 122, 173,  31, 238,  64, 101, 129,  93,   7, 206,  16,
  164,  99, 217,  72, 215, 178, 241, 174,  37, 228, 220,  90,   0, 222, 180, 199,
  186, 145, 159, 101,  67,  95, 139, 193,  56, 134, 147,  23, 106, 115, 125,  43,
  14, 250,  38,  28, 212,  74, 249, 190,  21, 132, 125,  25,  14,  86,  37, 244,
  220,  58, 200, 158, 179,  71, 171, 145, 249, 100,  23,  88, 115, 209,  40, 230,
  242,  86,  45, 244, 236,  58, 104, 157, 115,  79,  43, 162, 249, 206,  20, 164,
  123, 217,  24, 214,  82, 245, 238,  60, 100, 137,  91,  55, 218, 176, 223, 162,
  193, 206, 132, 167,  27, 210,  88, 237, 210, 108, 237, 106, 111, 125,  99,  15,
  74,  35, 188, 201, 138, 181,  63, 190, 129, 135,   5,  18,  30, 108,  69, 105,
  159, 117,  67,  63, 138, 129,  63,   6, 130,  23,  12, 114,  41,  44, 246, 234,
  54, 124, 181,  11, 190,  57, 132, 149,  27, 126,  91,   5, 218,  28, 220,  74,
  201, 190, 181, 135, 189,  17, 142, 101,  37,  94, 223, 198, 192, 150, 131, 119,
  11,  50,  58, 172, 159, 235,  64, 123, 129,  25,   6,  86,  21, 244, 124,  57,
  8, 150,  51, 116, 171,  59, 250, 152,   0,  80,  67, 225, 136,  69,  49, 158,
  165,  71, 223, 144, 193,  98, 135,  79,  19, 162, 105, 205, 116, 175,  59, 226,
  152,  79,  81, 163, 229, 200,  92, 177, 203, 164, 185, 219, 148, 217, 122, 215,
  30, 242,  70,  45, 148, 237, 122, 111,  29,  98,  79,  77, 163, 173, 201, 236,
  180, 107, 185, 121, 151,  21, 114, 127,  45,   2, 238,  14, 100,  37,  88, 223,
  210, 192, 238, 130, 103,  13,  82,  47, 236, 224, 106,  65, 125, 135,  13,  18,
  46, 108, 229, 106,  95, 125, 195,  12, 138,  43,  60, 250, 138,  31,  60,  66,
  137, 143,  53,  34, 190, 207, 134, 161,  23, 198, 112, 149,  35, 126, 203,   6,
  186,  23, 156, 113,  73,  39, 182, 209, 182, 229, 182,  93, 181, 205, 188, 173,
  139, 237,  56, 110, 145, 103, 103,  83,  83, 235, 232, 120, 113,  17,  38, 102,
  213,  86, 255, 246,   0,  52,   0, 184,   3, 144,   9,  96,  55,  64, 179, 131,
  169,   9, 246,  52,  52, 184, 187, 147, 153, 105,  87, 119, 243,  48,  42, 160,
  255, 194,   0, 140,   0,  40,  10, 240,  62,  32, 132, 195,  26, 138,  95,  61,
  194, 140, 143,  43,  34, 250, 206,  30, 164,  71, 217, 144, 213,  98, 255,  78,
  3, 164,   9, 216,  52, 208, 186, 227, 158,  73,  69, 183, 157, 177,  77, 167,
  173, 209, 236, 228, 106,  89, 125, 215,  12, 242,  42,  44, 252, 234,  10, 124,
  61,   8, 142,  51,  36, 170, 219, 254, 216,   6, 208,  22, 224, 118,  65,  53,
  134, 189,  23, 142, 113,  37,  38, 222, 214, 198, 246, 150,  55, 116, 179,  59,
  170, 153, 255,  84,   3, 248,   8,  16,  48,  96, 161,  67, 199, 136, 145,  51,
  102, 171,  87, 251, 240,  24,  32,  80, 193, 226, 132,  79,  25, 162,  85, 205,
  252, 172,  11, 232,  56, 112, 145,  35, 102, 203,  86, 187, 247, 152,  49,  80,
  167, 227, 208,  72, 225, 178,  69, 173, 157, 237,  76, 111, 169,  97, 247,  68,
  51, 152, 169,  83, 247, 232,  48, 112, 161,  35, 198, 200, 150, 179, 119, 171,
  49, 250, 164,  31, 216,  64, 209, 130, 229,  14,  94,  37, 196, 220, 154, 203
};

/// Main processing method
void Dvbt1ScramblerComponent::process()
{
  // request input
  DataSet< uint8_t >* in = NULL;
  getInputDataSet("input1", in);
  int size = in ? (int) in->data.size() : 0;

  // print debug info
  if(debug_x)
    LOG(LINFO) << "in/out: " << size << "/" << size;

  // request output - same size
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, size);
 
  // do the scrambling using the static array above
  for(ByteVecIt init = in->data.begin(), outit = out->data.begin(); init < in->data.end(); init++, outit++) {
    *outit = *init ^ scramblerPrbs_[scramblerOffset_];
    if (++scramblerOffset_ == 1504)
      scramblerOffset_ = 0;
  }
		
  // Copy the timestamp and sample rate for the DataSets
  out->timeStamp = in->timeStamp;
  out->sampleRate = in->sampleRate;

  // release input and output
  releaseInputDataSet("input1", in);
  releaseOutputDataSet("output1", out);

  // print the calculated bitrate
  if(reportInterval_x)
  {
    ptime t = microsec_clock::local_time(); // current time
    doneBytes_ += size; // increase processed bytes since last report
    time_duration delta = t-start_; // time elapsed from last report
    if(delta > seconds(reportInterval_x))
    {
      // interval is triggered, compute speed and report
      LOG(LINFO) << "Current TS bitrate: " << 8.0 * (double) doneBytes_ / (delta.total_microseconds()) << " Mbps";
      // reset counters
      start_ = t;
      doneBytes_ = 0;
    }
  }
}

/// Actions taken when the parameters change
/// 
/// This block is not to be reset if parameters change
void Dvbt1ScramblerComponent::parameterHasChanged(std::string name)
{
  if(name == "???")
  {
    destroy();
    setup();
  }
}

/// Set up counters, offsets, etc.
void Dvbt1ScramblerComponent::setup()
{
  scramblerOffset_ = 0;
  start_ = microsec_clock::local_time();
  doneBytes_ = 0L;
}

/// Destroy the component
void Dvbt1ScramblerComponent::destroy()
{
}

} // namespace phy
} // namespace iris
