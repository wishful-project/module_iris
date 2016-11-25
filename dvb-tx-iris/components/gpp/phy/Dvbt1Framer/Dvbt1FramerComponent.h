/**
 * \file components/gpp/phy/Dvbt1Framer/Dvbt1FramerComponent.h
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
 * The Dvbt1Framer component.
 */

#ifndef PHY_DVBT1FRAMERCOMPONENT_H_
#define PHY_DVBT1FRAMERCOMPONENT_H_

#include <boost/scoped_ptr.hpp>
#include "irisapi/PhyComponent.h"

#define T1_BLOCKS_PER_FRAME	68
#define T1_FRAMES_PER_SUPERFRAME	4

namespace iris
{
namespace phy
{

/** A DVB-T1 framer component.
 *
 * Dvbt1FramerComponent is the ninth block composing the DVB-T transmission chain.
 * The framer has the task to assemble together QAM data cells, pilot data cells,
 * and transmission parameters signaling (TPS) data cells into a frame structure
 * that will be mapped onto OFDM symbols.
 *
 * \image html framing.png DVB-T framing structure.
 * \image latex framing.png DVB-T framing structure.
 *
 * The basic frame structure starts from the OFDM symbol: 68 OFDM symbols constitute
 * one frame, and 4 frames build up a superframe.
 * Each OFDM symbol is composed by an useful portion, which comes from an IFFT operation,
 * and by a cyclic prefix (CP). 
 * The carriers of the useful portion are composed by active and null carriers,
 * which are switched off and are guard bands.
 *
 * \image html pilots.png DVB-T pilots structure.
 * \image latex pilots.png DVB-T pilots structure.
 *
 * Pilot carriers are divided between continual pilots, which occur on every
 * OFDM symbol at the same carrier position, and scattered pilots, which are
 * cyclically shifted of three positions at each new OFDM symbol. Additionally,
 * there are a number of carriers that are used to convey TPS data, useful for
 * purposes of frame synchronization and signalling. As displayed in the figure
 * above, the carriers are not created at the same power: while data and TPS
 * carriers have a unitary power, all pilot carriers are transmitted at a power
 * of 16/9.
 *
 * This block accepts in input complex float values and
 * generates in output complex float values. The block is capable to generate
 * internally all the required frame timing and modulation for the pilot and
 * TPS cells.
 *
 * There are several parameters that can be changed in the XML
 * configuration file:
 *
 * * _debug_: by default set to "false", is used to print some small debugging
 *          information for the interested developer.
 * * _hpcoderate_: by default set to "34", this is used to select one of the five
 *               possible coding rates. The admitted values are "12", "23", "34",
 *               "56", and "78", which are easily recognizable as the real coding
 *               ratioes written without the separating slash. This parameter
 *               refers to the high priority (HP) stream in case of hyerarchical
 *               transmission, differently it refers to the coderate of the
 *               single stream for nonhyerarchical transmission.
 * * _lpcoderate_: by default set to "34", this is used to select one of the five
 *               possible coding rates. The admitted values are "12", "23", "34",
 *               "56", and "78", which are easily recognizable as the real coding
 *               ratioes written without the separating slash. This parameter
 *               refers to the low priority (LP) stream in case of hyerarchical
 *               transmission, differently it is not used for nonhyerarchical
 *               transmission.
 * * _qammapping_: by default set to "16", this is used to select one of the three
 *               possible QAM mappings. The admitted values are "4", "16", "64".
 * * _hyerarchymode_: by default set to "0", which means "not hyerarchical".
 *                    Hierarchical modes are used to transmit two different transport
 *                    streams, one with a high priority (HP) information and 
 *                    another one with a low priority (LP) information. The admitted
 *                    values are "0, "1", "2", "4". NOTE: hyerarchical modes 
 *                    are not implemented in the current release of
 *                    this modulator.
 * * _ofdmmode_: by default set to "2048", this is used to select one of the three
 *               possible OFDM modes. The admitted values are "2048", "4096",
 *               "8192", respectively for 2K, 4K (DVB-H, unused), and 8K.
 * * _deltamode_: by default set to "32", this is used to select one of the four
 *                possible cyclic prefix lengths. The admitted values are "32", 
 *                "16", "8", and "4", which are directly derived from the
 *                denominator of the cyclic prefix fraction (1/32, 1/16, 1/8, 1/4).
 * * _cellid_: by default set to "-1", which means it is disabled. The Cell
 *             Identifier is used to identify transmission towers with a
 *             16 bit numeric identifier, and is used only in case of DVB-H transmission.
 *             NOTE: DVB-H is not implemented in this software modulator.
 * * _indepthinterleaver_: by default set to "false", which means it is disabled.
 *                         This additional interleaver is used only in DVB-H mode
 *                         and should improve the diversity of the received signal
 *                         in case of transmission over time varying channels.
 *                         NOTE: DVB-H is not implemented in this software modulator.
 *
 * __References__
 * * ETSI Standard: _EN 300 744 V1.5.1, Digital Video Broadcasting (DVB); Framing
 *   structure, channel coding and modulation for digital terrestrial television_,
 *   available at [ETSI Publications Download Area](http://pda.etsi.org/pda/queryform.asp)
 */
class Dvbt1FramerComponent
  : public PhyComponent
{
 public:

  /// A vector of bytes
  typedef std::vector<uint8_t>  ByteVec;
  
  /// An iterator for a vector of bytes
  typedef ByteVec::iterator     ByteVecIt;

  /// A complex type
  typedef std::complex<float>   Cplx;

  /// A vector of complex
  typedef std::vector<Cplx>     CplxVec;

  // An iterator for a vector of complex
  typedef CplxVec::iterator     CplxVecIt;

  Dvbt1FramerComponent(std::string name);
  ~Dvbt1FramerComponent();
  virtual void calculateOutputTypes(
      std::map<std::string, int>& inputTypes,
      std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);

 private:

  bool debug_x;               ///< Debug flag (default = false)
  int ofdmMode_x;             ///< OFDM mode (default = 2048)
  int cellId_x;               ///< Cell ID for DVB-H mode (default = -1)
  int qamMapping_x;           ///< QAM constellation mapping (default = 16)
  bool inDepthInterleaver_x;  ///< In-depth interleaver for DVB-H mode (default = false)
  int hyerarchyMode_x;        ///< Hyerarchical mode (default = 0)
  int hpCodeRate_x;           ///< HP stream channel coding rate (default = 34) 
  int lpCodeRate_x;           ///< LP stream channel coding rate (default = 34) 
  int deltaMode_x;            ///< Cyclic prefix ratio (default = 32)

  void setup();
  void destroy();

  double timeStamp_;          ///< Timestamp of current frame
  double sampleRate_;         ///< Sample rate of current frame

  int nMax_;                  ///< data carriers
  int kMax_;                  ///< active carriers
  int fraOffset_;             ///< framer offset
  CplxVec fraRegister_;       ///< framer template
  
  int blockIndex_;            ///< OFDM block index
  float tpsAmpl_[6817];       ///< tps_amplitudes
  uint8_t tps_[T1_BLOCKS_PER_FRAME]; ///< tps data

  static int cont_pilot_position[178];
  static int tps_position[69];
  static unsigned char prbs_pilot[6817];

  int t1_tps_generate(unsigned char *tps, int block_in_frame, int frame_in_superframe);
   			
  /// Useful templates
  template <typename T, size_t N>
  static T* begin(T(&arr)[N]) { return &arr[0]; }
  template <typename T, size_t N>
  static T* end(T(&arr)[N]) { return &arr[0]+N; }

};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1FRAMERCOMPONENT_H_
