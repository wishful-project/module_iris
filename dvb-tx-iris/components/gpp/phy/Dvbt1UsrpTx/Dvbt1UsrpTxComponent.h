/**
 * \file components/gpp/phy/Dvbt1UsrpTx/Dvbt1UsrpTxComponent.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
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
 * The Dvbt1UsrpTx component.
 */

#ifndef PHY_DVBT1USRPTXCOMPONENT_H_
#define PHY_DVBT1USRPTXCOMPONENT_H_

#include "irisapi/PhyComponent.h"
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/thread/condition_variable.hpp>

namespace iris
{
namespace phy
{

/** The Dvbt1UsrpTx component.
 * 
 * A sink component which writes to a USRP transmitter using the
 * Universal Hardware Driver (UHD).
 * This component supports streaming data by default. This component is derived
 * from the component used by the Iris modules. In addition to that, we use here
 * additional threading and buffering to allow high data rate streams to be 
 * continuously and uninterruptedly transmitted from the input coming from other
 * blocks to the real USRP device.
 *
 * This block accepts in input complex float values.
 *
 * There are several parameters that can be changed in the XML
 * configuration file:
 *
 * * _args_: by default set to "". This is a string that can be used to address
 *           one particular USRP device by specifying its IP address.
 * * _rate_: by default set to "1000000", it represents the sampling rate at which
 *           the digital samples are sent to the device.
 * * _frequency_: by default set to "2400000000", it is the frequency at which
 *                the BB signal, after digital to analog conversion, will be
 *                modulated.
 * * _gain_: by default set to "1", it is the gain of the final amplifier in the
 *           USRP TX chain.
 * * _streaming_: by default set to true, it states whether a continuous stream
 *                of samples has to be expected.
 * * _fixlooffset_: by default set to "0", this is the offset at which the analog
 *                  oscillator will up-convert the BB signal, with respect to the
 *                  specified frequency. This offset is recovered in digital by
 *                  means of a digitally controlled oscillator implemented in the
 *                  USRP FPGA.
 * * _antenna_: by default set to "", which means automatic selection. This parameter
 *              can be used to select one particular antenna, if more than one is
 *              present.
 * * _subdev_: by default set to "", which means automatic selection. This parameter
 *             allows selecting one particular subdevice inside of the device, if
 *             more than one is present.
 * * _bw_: by default set to "0" Hz, which means automatic selection. This parameter
 *         selects the bandwidth of the daughterboard IF filter, if present.
 * * _ref_: can be one of "internal", "external", "mimo", by default it is set to
 *          "internal". This parameter represents the type of clock reference signal
 *          used for the synchronization of all the device chips.
 * * _fmt_: can be one of "fc64" (double), "fc32" (float) or "sc16" (short),
 *          by default set to "fc32".
 *          This is the default sample precision used for sample representation.
 * * _numbuffers_: by default set to "4". A number of internal buffers is required
 *                 to temporarily store the samples that are sent to the USRP.
 *                 If buffers are not used, there could be moments during which
 *                 the USRP (which runs inside of an asynchronous thread) is lacking
 *                 input samples, thus degrading the quality of the emitted signal.
 * * _buffersize_: by default set to "1000000" samples, this is the number of
 *                 samples contained in one of the buffers mentioned above.
 */
class Dvbt1UsrpTxComponent
  : public PhyComponent
{
public:
  Dvbt1UsrpTxComponent(std::string name);
  virtual ~Dvbt1UsrpTxComponent();
  virtual void calculateOutputTypes(
    std::map<std::string, int>& inputTypes,
    std::map<std::string, int>& outputTypes);
  virtual void registerPorts();
  virtual void initialize();
  virtual void process();
  virtual void parameterHasChanged(std::string name);
  void usrpThreadProcedure();

private:
    //Exposed parameters
  std::string args_x;   //!< See http://files.ettus.com/uhd_docs/manual/html/identification.html
  double rate_x;        //!< Rate of outgoing samples
  double frequency_x;   //!< Tx frequency
  double fixLoOffset_x; //!< Fix the local oscillator offset (defaults to 2*rate)
  float gain_x;         //!< Overall tx gain
  std::string antenna_x;//!< Daughterboard antenna selection
  std::string subDev_x; //!< Daughterboard subdevice specification
  double bw_x;          //!< Daughterboard IF filter bandwidth (Hz)
  std::string ref_x;    //!< Reference waveform (internal, external, mimo)
  bool streaming_x;     //!< Streaming or bursty traffic?
  std::string fmt_x;    //!< Data format (fc64, fc32 or sc16)
  int bufferSize_x;     //!< Size (in samples) of a single buffer
  int numBuffers_x;     //!< Number of buffers

  ReadBuffer< std::complex<float> >* inBuf_; ///< Convenience pointer to input buffer.
  uhd::usrp::multi_usrp::sptr usrp_;  ///< The device.
  uhd::tx_streamer::sptr txStream_;
  std::vector< std::vector< std::complex<float> > > bufs_;
  std::vector<int> fulls_;
  boost::condition_variable condW_, condR_;
  boost::mutex mutW_, mutR_;
  boost::mutex mut_;
  int currentRead_, currentWrite_; 
  bool runUsrp_;
  boost::thread *usrpThread_;
};

} // namespace phy
} // namespace iris

#endif // PHY_DVBT1USRPTXCOMPONENT_H_
