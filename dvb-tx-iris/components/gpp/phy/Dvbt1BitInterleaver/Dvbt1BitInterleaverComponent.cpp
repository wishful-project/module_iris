/**
 * \file components/gpp/phy/Dvbt1BitInterleaver/Dvbt1BitInterleaverComponent.cpp
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
 * Implementation of the Dvbt1BitInterleaver component.
 */

#include "Dvbt1BitInterleaverComponent.h"

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
IRIS_COMPONENT_EXPORTS(PhyComponent, Dvbt1BitInterleaverComponent);

/// Default constructor
///
/// Registers the block parameters and initializes some variables
Dvbt1BitInterleaverComponent::Dvbt1BitInterleaverComponent(std::string name)
  : PhyComponent(name,                            // component name
                "dvbt1bitinterleaver",                 // component type
                "A DVB-T1 bit interleaver component", // description
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
    
  intRegister_[0] = NULL;
  intRegister_[1] = NULL;
}

/// Default destructor
///
/// Just calls destroy().
Dvbt1BitInterleaverComponent::~Dvbt1BitInterleaverComponent()
{
  destroy();
}

/// Register the interleaver ports with the IRIS system
///
/// This component has two inputs that accept bits (one bit per byte) and one
/// output that provides symbols (in one byte).
void Dvbt1BitInterleaverComponent::registerPorts()
{
  registerInputPort("input1", TypeInfo< uint8_t >::identifier);
  registerInputPort("input2", TypeInfo< uint8_t >::identifier);
  registerOutputPort("output1", TypeInfo< uint8_t >::identifier);
}

/// Calculate the output port types for the IRIS system
///
/// The single output port must provide bytes.
void Dvbt1BitInterleaverComponent::calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes)
{
  outputTypes["output1"] = TypeInfo< uint8_t >::identifier;
}

/// Initialize the component
///
/// Just calls setup().
void Dvbt1BitInterleaverComponent::initialize()
{
  setup();
}

/// The interleaving addresses for QPSK
int Dvbt1BitInterleaverComponent::address_v2[252] = {
	0, 127,   2, 129,   4, 131,   6, 133,   8, 135,  10, 137,  12, 139,  14, 141,  16, 143,  18, 145,  20, 147,
	22, 149,  24, 151,  26, 153,  28, 155,  30, 157,  32, 159,  34, 161,  36, 163,  38, 165,  40, 167,  42, 169,
	44, 171,  46, 173,  48, 175,  50, 177,  52, 179,  54, 181,  56, 183,  58, 185,  60, 187,  62, 189,  64, 191,
	66, 193,  68, 195,  70, 197,  72, 199,  74, 201,  76, 203,  78, 205,  80, 207,  82, 209,  84, 211,  86, 213,
	88, 215,  90, 217,  92, 219,  94, 221,  96, 223,  98, 225, 100, 227, 102, 229, 104, 231, 106, 233, 108, 235,
	110, 237, 112, 239, 114, 241, 116, 243, 118, 245, 120, 247, 122, 249, 124, 251, 126,   1, 128,   3, 130,   5,
	132,   7, 134,   9, 136,  11, 138,  13, 140,  15, 142,  17, 144,  19, 146,  21, 148,  23, 150,  25, 152,  27,
	154,  29, 156,  31, 158,  33, 160,  35, 162,  37, 164,  39, 166,  41, 168,  43, 170,  45, 172,  47, 174,  49,
	176,  51, 178,  53, 180,  55, 182,  57, 184,  59, 186,  61, 188,  63, 190,  65, 192,  67, 194,  69, 196,  71,
	198,  73, 200,  75, 202,  77, 204,  79, 206,  81, 208,  83, 210,  85, 212,  87, 214,  89, 216,  91, 218,  93,
	220,  95, 222,  97, 224,  99, 226, 101, 228, 103, 230, 105, 232, 107, 234, 109, 236, 111, 238, 113, 240, 115,
	242, 117, 244, 119, 246, 121, 248, 123, 250, 125
};

/// The interleaving addresses for 16-QAM
int Dvbt1BitInterleaverComponent::address_v4[504] = {
	0, 254, 421, 171,   4, 258, 425, 175,   8, 262, 429, 179,  12, 266, 433, 183,  16, 270, 437, 187,  20, 274,
	441, 191,  24, 278, 445, 195,  28, 282, 449, 199,  32, 286, 453, 203,  36, 290, 457, 207,  40, 294, 461, 211,
	44, 298, 465, 215,  48, 302, 469, 219,  52, 306, 473, 223,  56, 310, 477, 227,  60, 314, 481, 231,  64, 318, 
	485, 235,  68, 322, 489, 239,  72, 326, 493, 243,  76, 330, 497, 247,  80, 334, 501, 251,  84, 338,   1, 255,  
	88, 342,   5, 259,  92, 346,   9, 263,  96, 350,  13, 267, 100, 354,  17, 271, 104, 358,  21, 275, 108, 362,  
	25, 279, 112, 366,  29, 283, 116, 370,  33, 287, 120, 374,  37, 291, 124, 378,  41, 295, 128, 382,  45, 299, 
	132, 386,  49, 303, 136, 390,  53, 307, 140, 394,  57, 311, 144, 398,  61, 315, 148, 402,  65, 319, 152, 406,  
	69, 323, 156, 410,  73, 327, 160, 414,  77, 331, 164, 418,  81, 335, 168, 422,  85, 339, 172, 426,  89, 343, 
	176, 430,  93, 347, 180, 434,  97, 351, 184, 438, 101, 355, 188, 442, 105, 359, 192, 446, 109, 363, 196, 450, 
	113, 367, 200, 454, 117, 371, 204, 458, 121, 375, 208, 462, 125, 379, 212, 466, 129, 383, 216, 470, 133, 387, 
	220, 474, 137, 391, 224, 478, 141, 395, 228, 482, 145, 399, 232, 486, 149, 403, 236, 490, 153, 407, 240, 494, 
	157, 411, 244, 498, 161, 415, 248, 502, 165, 419, 252,   2, 169, 423, 256,   6, 173, 427, 260,  10, 177, 431, 
	264,  14, 181, 435, 268,  18, 185, 439, 272,  22, 189, 443, 276,  26, 193, 447, 280,  30, 197, 451, 284,  34, 
	201, 455, 288,  38, 205, 459, 292,  42, 209, 463, 296,  46, 213, 467, 300,  50, 217, 471, 304,  54, 221, 475, 
	308,  58, 225, 479, 312,  62, 229, 483, 316,  66, 233, 487, 320,  70, 237, 491, 324,  74, 241, 495, 328,  78, 
	245, 499, 332,  82, 249, 503, 336,  86, 253,   3, 340,  90, 257,   7, 344,  94, 261,  11, 348,  98, 265,  15, 
	352, 102, 269,  19, 356, 106, 273,  23, 360, 110, 277,  27, 364, 114, 281,  31, 368, 118, 285,  35, 372, 122, 
	289,  39, 376, 126, 293,  43, 380, 130, 297,  47, 384, 134, 301,  51, 388, 138, 305,  55, 392, 142, 309,  59, 
	396, 146, 313,  63, 400, 150, 317,  67, 404, 154, 321,  71, 408, 158, 325,  75, 412, 162, 329,  79, 416, 166, 
	333,  83, 420, 170, 337,  87, 424, 174, 341,  91, 428, 178, 345,  95, 432, 182, 349,  99, 436, 186, 353, 103, 
	440, 190, 357, 107, 444, 194, 361, 111, 448, 198, 365, 115, 452, 202, 369, 119, 456, 206, 373, 123, 460, 210, 
	377, 127, 464, 214, 381, 131, 468, 218, 385, 135, 472, 222, 389, 139, 476, 226, 393, 143, 480, 230, 397, 147, 
	484, 234, 401, 151, 488, 238, 405, 155, 492, 242, 409, 159, 496, 246, 413, 163, 500, 250, 417, 167
};

/// The interleaving addresses for 64-QAM
int Dvbt1BitInterleaverComponent::address_v6[756] = {
	0, 381, 631, 256, 128, 509,   6, 387, 637, 262, 134, 515,  12, 393, 643, 268, 140, 521,  18, 399, 649, 274, 
	146, 527,  24, 405, 655, 280, 152, 533,  30, 411, 661, 286, 158, 539,  36, 417, 667, 292, 164, 545,  42, 423, 
	673, 298, 170, 551,  48, 429, 679, 304, 176, 557,  54, 435, 685, 310, 182, 563,  60, 441, 691, 316, 188, 569,  
	66, 447, 697, 322, 194, 575,  72, 453, 703, 328, 200, 581,  78, 459, 709, 334, 206, 587,  84, 465, 715, 340, 
	212, 593,  90, 471, 721, 346, 218, 599,  96, 477, 727, 352, 224, 605, 102, 483, 733, 358, 230, 611, 108, 489, 
	739, 364, 236, 617, 114, 495, 745, 370, 242, 623, 120, 501, 751, 376, 248, 629, 126, 507,   1, 382, 254, 635, 
	132, 513,   7, 388, 260, 641, 138, 519,  13, 394, 266, 647, 144, 525,  19, 400, 272, 653, 150, 531,  25, 406, 
	278, 659, 156, 537,  31, 412, 284, 665, 162, 543,  37, 418, 290, 671, 168, 549,  43, 424, 296, 677, 174, 555,  
	49, 430, 302, 683, 180, 561,  55, 436, 308, 689, 186, 567,  61, 442, 314, 695, 192, 573,  67, 448, 320, 701, 
	198, 579,  73, 454, 326, 707, 204, 585,  79, 460, 332, 713, 210, 591,  85, 466, 338, 719, 216, 597,  91, 472, 
	344, 725, 222, 603,  97, 478, 350, 731, 228, 609, 103, 484, 356, 737, 234, 615, 109, 490, 362, 743, 240, 621, 
	115, 496, 368, 749, 246, 627, 121, 502, 374, 755, 252, 633, 127, 508, 380,   5, 258, 639, 133, 514, 386,  11, 
	264, 645, 139, 520, 392,  17, 270, 651, 145, 526, 398,  23, 276, 657, 151, 532, 404,  29, 282, 663, 157, 538, 
	410,  35, 288, 669, 163, 544, 416,  41, 294, 675, 169, 550, 422,  47, 300, 681, 175, 556, 428,  53, 306, 687, 
	181, 562, 434,  59, 312, 693, 187, 568, 440,  65, 318, 699, 193, 574, 446,  71, 324, 705, 199, 580, 452,  77, 
	330, 711, 205, 586, 458,  83, 336, 717, 211, 592, 464,  89, 342, 723, 217, 598, 470,  95, 348, 729, 223, 604, 
	476, 101, 354, 735, 229, 610, 482, 107, 360, 741, 235, 616, 488, 113, 366, 747, 241, 622, 494, 119, 372, 753, 
	247, 628, 500, 125, 378,   3, 253, 634, 506, 131, 384,   9, 259, 640, 512, 137, 390,  15, 265, 646, 518, 143, 
	396,  21, 271, 652, 524, 149, 402,  27, 277, 658, 530, 155, 408,  33, 283, 664, 536, 161, 414,  39, 289, 670, 
	542, 167, 420,  45, 295, 676, 548, 173, 426,  51, 301, 682, 554, 179, 432,  57, 307, 688, 560, 185, 438,  63, 
	313, 694, 566, 191, 444,  69, 319, 700, 572, 197, 450,  75, 325, 706, 578, 203, 456,  81, 331, 712, 584, 209, 
	462,  87, 337, 718, 590, 215, 468,  93, 343, 724, 596, 221, 474,  99, 349, 730, 602, 227, 480, 105, 355, 736, 
	608, 233, 486, 111, 361, 742, 614, 239, 492, 117, 367, 748, 620, 245, 498, 123, 373, 754, 626, 251, 504, 129, 
	379,   4, 632, 257, 510, 135, 385,  10, 638, 263, 516, 141, 391,  16, 644, 269, 522, 147, 397,  22, 650, 275, 
	528, 153, 403,  28, 656, 281, 534, 159, 409,  34, 662, 287, 540, 165, 415,  40, 668, 293, 546, 171, 421,  46, 
	674, 299, 552, 177, 427,  52, 680, 305, 558, 183, 433,  58, 686, 311, 564, 189, 439,  64, 692, 317, 570, 195, 
	445,  70, 698, 323, 576, 201, 451,  76, 704, 329, 582, 207, 457,  82, 710, 335, 588, 213, 463,  88, 716, 341, 
	594, 219, 469,  94, 722, 347, 600, 225, 475, 100, 728, 353, 606, 231, 481, 106, 734, 359, 612, 237, 487, 112, 
	740, 365, 618, 243, 493, 118, 746, 371, 624, 249, 499, 124, 752, 377, 630, 255, 505, 130,   2, 383, 636, 261, 
	511, 136,   8, 389, 642, 267, 517, 142,  14, 395, 648, 273, 523, 148,  20, 401, 654, 279, 529, 154,  26, 407, 
	660, 285, 535, 160,  32, 413, 666, 291, 541, 166,  38, 419, 672, 297, 547, 172,  44, 425, 678, 303, 553, 178,  
	50, 431, 684, 309, 559, 184,  56, 437, 690, 315, 565, 190,  62, 443, 696, 321, 571, 196,  68, 449, 702, 327, 
	577, 202,  74, 455, 708, 333, 583, 208,  80, 461, 714, 339, 589, 214,  86, 467, 720, 345, 595, 220,  92, 473, 
	726, 351, 601, 226,  98, 479, 732, 357, 607, 232, 104, 485, 738, 363, 613, 238, 110, 491, 744, 369, 619, 244, 
	116, 497, 750, 375, 625, 250, 122, 503
};

/// Main processing method
void Dvbt1BitInterleaverComponent::process()
{
  // request input
  DataSet< uint8_t > *in1 = NULL;
  getInputDataSet("input1", in1);

  // calculate sizes
  int in1size = in1 ? (int) in1->data.size() : 0;
  int outsize = intLength_[0] * ((in1size + intOffset_[0]) / intLength_[0]) / nu_;
  
  // request output
  DataSet< uint8_t >* out = NULL;
  getOutputDataSet("output1", out, outsize);
 		
  // print debug info
  if(debug_x)
    LOG(LINFO) << "in1/out: " << in1size << "/" << outsize;
    
  // bit by bit
  for(ByteVecIt in1it = in1->data.begin(), outit = out->data.begin();
    in1it < in1->data.end(); in1it++)
  {
    // copy to register
    intRegister_[0][intOffset_[0]++] = *in1it;
    
    // trigger interleaving
    if(intOffset_[0] == intLength_[0])
    {
      // reset offset
      intOffset_[0] = 0;
      
      // do the copy
      if(hyerarchyMode_x == 0)
      {
        switch(qamMapping_x)
        {
          // read back according to the QAM mode and compose the output symbol
          case 4:
            for(int b = 0; b < intLength_[0]; b += 2)
            {
              *outit++ =
                (intRegister_[0][address_v2[b + 0]] << 1) |
                (intRegister_[0][address_v2[b + 1]] << 0);
            }
            break;
          case 16:
            for(int b = 0; b < intLength_[0]; b += 4)
            {
              *outit++ =
                (intRegister_[0][address_v4[b + 0]] << 3) |
                (intRegister_[0][address_v4[b + 1]] << 2) |
                (intRegister_[0][address_v4[b + 2]] << 1) |
                (intRegister_[0][address_v4[b + 3]] << 0);
            }
            break;
          case 64:
            for(int b = 0; b < intLength_[0]; b += 6)
            {
              *outit++ =
                (intRegister_[0][address_v6[b + 0]] << 5) |
                (intRegister_[0][address_v6[b + 1]] << 4) |
                (intRegister_[0][address_v6[b + 2]] << 3) |
                (intRegister_[0][address_v6[b + 3]] << 2) |
                (intRegister_[0][address_v6[b + 4]] << 1) |
                (intRegister_[0][address_v6[b + 5]] << 0);
            }
            break;
          default:
            LOG(LERROR) << "Unsupported QAM mapping";
        }
      }
    }
  }
    
  // copy the timestamp and sample rate for the DataSets
  out->timeStamp = in1->timeStamp;
  out->sampleRate = in1->sampleRate;

  // release input and output
  releaseInputDataSet("input1", in1);
  releaseOutputDataSet("output1", out);
}

/// Actions taken when the parameters change
/// 
/// This block has two significant parameters
void Dvbt1BitInterleaverComponent::parameterHasChanged(std::string name)
{
  if(name == "qammapping" || name == "hyerarchymode")
  {
    destroy();
    setup();
  }
}

/// Set up all offsets, clean registers
void Dvbt1BitInterleaverComponent::setup()
{
  // clean
  intOffset_[0] = 0;
  intOffset_[1] = 0;
  
  // modulation order
  nu_ = qamMapping_x == 4 ? 2 : (qamMapping_x == 16 ? 4 : 6);
  
  // lengths
  intLength_[0] = (hyerarchyMode_x == 0 ? (126 * nu_) : 126 * 2);
  intLength_[1] = (hyerarchyMode_x == 0 ? 1 /* to avoid divide by 0 error */
    : (126 * (nu_ - 2)));
    
  // alloc
  intRegister_[0] = new uint8_t [intLength_[0]];
  intRegister_[1] = new uint8_t [intLength_[1]];
}

/// Destroy the component
void Dvbt1BitInterleaverComponent::destroy()
{
  // clean
  delete[] intRegister_[0];
  delete[] intRegister_[1];
}

} // namespace phy
} // namespace iris
