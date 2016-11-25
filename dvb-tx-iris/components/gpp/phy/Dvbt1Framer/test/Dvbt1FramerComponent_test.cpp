/**
 * \file components/gpp/phy/Dvbt1Framer/test/Dvbt1FramerComponent_test.cpp
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
 * Main test file for Dvbt1Framer component.
 */

#define BOOST_TEST_MODULE Dvbt1FramerComponent_test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>

#include "../Dvbt1FramerComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;

BOOST_AUTO_TEST_SUITE (Dvbt1FramerComponent_Test)

BOOST_AUTO_TEST_CASE(Dvbt1FramerComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(Dvbt1FramerComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(Dvbt1FramerComponent_Parm_Test)
{
  Dvbt1FramerComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("hpcoderate") == "34");
  BOOST_CHECK(mod.getParameterDefaultValue("lpcoderate") == "34");
  BOOST_CHECK(mod.getParameterDefaultValue("qammapping") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("hyerarchymode") == "0");
  BOOST_CHECK(mod.getParameterDefaultValue("ofdmmode") == "2048");
  BOOST_CHECK(mod.getParameterDefaultValue("deltamode") == "32");
  BOOST_CHECK(mod.getParameterDefaultValue("cellid") == "-1");
  BOOST_CHECK(mod.getParameterDefaultValue("indepthinterleaver") == "false");
}

BOOST_AUTO_TEST_CASE(Dvbt1FramerComponent_Ports_Test)
{
  typedef std::complex<float>   Cplx;
  
  Dvbt1FramerComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts[0].portName == "input1");
  BOOST_REQUIRE(iPorts[0].supportedTypes.front() ==
      TypeInfo< Cplx >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< Cplx >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< Cplx >::identifier);
}

BOOST_AUTO_TEST_CASE(Dvbt1FramerComponent_Init_Test)
{
  typedef std::complex<float>   Cplx;

  Dvbt1FramerComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(Dvbt1FramerComponent_Process_Test)
{
  
  int ofdmarr[] = {2048,4096,8192};
  string ofdmstringarr[] = {"2048","4096","8192"};
  int Narr[] = {1512,3024,6048};
  int Karr[] = {1705,3409,6817};
  typedef std::complex<float>   Cplx;
  for(int oi = 0; oi < sizeof(ofdmarr) / sizeof(ofdmarr[0]); oi++)
  { 
      // generate random data
      if(true)
      {
        Dvbt1FramerComponent mod("test");
        mod.setValue("ofdmmode", ofdmarr[oi]);
        mod.registerPorts();

        map<string, int> iTypes,oTypes;
        iTypes["input1"] = TypeInfo< Cplx >::identifier;
        mod.calculateOutputTypes(iTypes,oTypes);

        DataBufferTrivial<Cplx> in;
        DataBufferTrivial<Cplx> out;

        // Create enough data
        DataSet< Cplx >* iSet = NULL;
        int num = Narr[oi] * (1000000 / Narr[oi]);
        in.getWriteData(iSet, num);
        boost::random::mt19937 rng;
        boost::random::normal_distribution<> gauss(0,1);
        for(int i=0;i<num;i++)
            iSet->data[i] = Cplx(gauss(rng), gauss(rng)); // random complex data
        in.releaseWriteData(iSet);

        mod.setBuffers(&in,&out);
        mod.initialize();
        BOOST_REQUIRE_NO_THROW(mod.process());

        // length check
        BOOST_REQUIRE(out.hasData());
        DataSet< Cplx >* oSet = NULL;
        out.getReadData(oSet);
        //printf("%d,%d: oSet->data.size()=%lu, num=%d\n", hyerarr[hi], qamarr[qi], oSet->data.size(), num);
        BOOST_CHECK(oSet->data.size() == Karr[oi] * (num / Narr[oi])); // good size
        out.releaseReadData(oSet);
      }

      // load MATLAB/OCTAVE files
      if(true)
      {
        Dvbt1FramerComponent mod("test");
        mod.setValue("ofdmmode", ofdmarr[oi]);
        mod.registerPorts();

        map<string, int> iTypes,oTypes;
        iTypes["input1"] = TypeInfo< Cplx >::identifier;
        mod.calculateOutputTypes(iTypes,oTypes);

        DataBufferTrivial<Cplx> in;
        DataBufferTrivial<Cplx> out, out2;

        // load data from input file
        DataSet< Cplx >* iSet = NULL;
        int num = RawFileUtility::getNumElements<Cplx>("input");
        in.getWriteData(iSet, num);
        RawFileUtility::read(iSet->data.begin(), iSet->data.end(), "input");
        in.releaseWriteData(iSet);

        // execute
        mod.setBuffers(&in,&out);
        mod.initialize();
        BOOST_REQUIRE_NO_THROW(mod.process());

        // length check
        BOOST_REQUIRE(out.hasData());
        DataSet< Cplx >* oSet = NULL;
        out.getReadData(oSet);
        BOOST_CHECK(oSet->data.size() == Karr[oi] * (num / Narr[oi])); // good size

        // load data from output file
        DataSet< Cplx >* oSet2 = NULL;
        int num2 = RawFileUtility::getNumElements<Cplx>("output" + ofdmstringarr[oi]);
        out2.getWriteData(oSet2, num2);
        RawFileUtility::read(oSet2->data.begin(), oSet2->data.end(), "output"
          + ofdmstringarr[oi]);
        BOOST_CHECK(oSet2->data.size() == oSet->data.size()); // good size

        // value check
        float tol = 1e-4, tol2 = tol * tol;
        for(int i=0;i<oSet2->data.size();i++)
        {
            float diff2 = (oSet->data[i].real() - oSet2->data[i].real()) *
              (oSet->data[i].real() - oSet2->data[i].real()) +
              (oSet->data[i].imag() - oSet2->data[i].imag()) *
              (oSet->data[i].imag() - oSet2->data[i].imag());
            BOOST_CHECK(diff2 < tol2);
            /*if(diff2 >= tol2)
            {
              printf("%d, file %d: %f, %f\n", ofdmarr[oi], i, oSet2->data[i].real(), oSet2->data[i].imag());
              printf("%d, iris %d: %f, %f\n", ofdmarr[oi], i, oSet->data[i].real(), oSet->data[i].imag());
            }*/
        }
        out2.releaseWriteData(oSet2);
        out.releaseReadData(oSet);
      }
  }
}

BOOST_AUTO_TEST_SUITE_END()
