/**
 * \file components/gpp/phy/Dvbt1BitInterleaver/test/Dvbt1BitInterleaverComponent_test.cpp
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
 * Main test file for Dvbt1BitInterleaver component.
 */

#define BOOST_TEST_MODULE Dvbt1BitInterleaverComponent_test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>

#include "../Dvbt1BitInterleaverComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;

BOOST_AUTO_TEST_SUITE (Dvbt1BitInterleaverComponent_Test)

BOOST_AUTO_TEST_CASE(Dvbt1BitInterleaverComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(Dvbt1BitInterleaverComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(Dvbt1BitInterleaverComponent_Parm_Test)
{
  Dvbt1BitInterleaverComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("qammapping") == "16");
  BOOST_CHECK(mod.getParameterDefaultValue("hyerarchymode") == "0");
}

BOOST_AUTO_TEST_CASE(Dvbt1BitInterleaverComponent_Ports_Test)
{
  Dvbt1BitInterleaverComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 2);
  BOOST_REQUIRE(iPorts[0].portName == "input1");
  BOOST_REQUIRE(iPorts[0].supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);
  BOOST_REQUIRE(iPorts[1].portName == "input2");
  BOOST_REQUIRE(iPorts[1].supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  iTypes["input2"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< uint8_t >::identifier);
}

BOOST_AUTO_TEST_CASE(Dvbt1BitInterleaverComponent_Init_Test)
{
  Dvbt1BitInterleaverComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  iTypes["input2"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(Dvbt1BitInterleaverComponent_Process_Test)
{
  
  int qamarr[] = {4,16,64};
  string qamarrs[] = {"4", "16", "64"};
  int nuarr[]  = {2,4,6};
  //int hyerarr[] = {0,1,2,4};
  //string hyerarrs[] = {"nh","h","h","h"};
  int hyerarr[] = {0};
  string hyerarrs[] = {"nh"};

  for(int hi = 0; hi < sizeof(hyerarr) / sizeof(hyerarr[0]); hi++)
  {
    for(int qi = 0; qi < sizeof(qamarr) / sizeof(qamarr[0]); qi++)
    {
      // generate random data
      if(true)
      {
        Dvbt1BitInterleaverComponent mod("test");
        mod.setValue("qammapping", qamarr[qi]);
        mod.setValue("hyerarchymode", hyerarr[hi]);
        mod.registerPorts();

        map<string, int> iTypes,oTypes;
        iTypes["input1"] = TypeInfo< uint8_t >::identifier;
        mod.calculateOutputTypes(iTypes,oTypes);

        DataBufferTrivial<uint8_t> in;
        DataBufferTrivial<uint8_t> out;

        // Create enough data
        DataSet< uint8_t >* iSet = NULL;
        int num = (126*nuarr[qi])*((4*((8*204*(10000000/204))/3))/(126*nuarr[qi]));
        in.getWriteData(iSet, num);
        boost::random::mt19937 rng;
        boost::random::uniform_int_distribution<> two(0,1);
        for(int i=0;i<num;i++)
        {
          iSet->data[i] = uint8_t(two(rng)); // random data
        }
        in.releaseWriteData(iSet);

        mod.setBuffers(&in,&out);
        mod.initialize();
        BOOST_REQUIRE_NO_THROW(mod.process());

        // length check
        BOOST_REQUIRE(out.hasData());
        DataSet< uint8_t >* oSet = NULL;
        out.getReadData(oSet);
        printf("%dQAM: oSet->data.size()=%lu, num=%d\n", qamarr[qi], oSet->data.size(), num);
        BOOST_CHECK(oSet->data.size() == num / nuarr[qi]); // good size
        out.releaseReadData(oSet);
      }

      // load MATLAB/OCTAVE files
      if(true)
      {
        Dvbt1BitInterleaverComponent mod("test");
        mod.setValue("qammapping", qamarr[qi]);
        mod.setValue("hyerarchymode", hyerarr[hi]);
        mod.registerPorts();

        map<string, int> iTypes,oTypes;
        iTypes["input1"] = TypeInfo< uint8_t >::identifier;
        mod.calculateOutputTypes(iTypes,oTypes);

        DataBufferTrivial<uint8_t> in;
        DataBufferTrivial<uint8_t> out, out2;

        // load data from input file
        DataSet< uint8_t >* iSet = NULL;
        int num = RawFileUtility::getNumElements<uint8_t>("input");
        in.getWriteData(iSet, num);
        RawFileUtility::read(iSet->data.begin(), iSet->data.end(), "input");
        in.releaseWriteData(iSet);

        // execute
        mod.setBuffers(&in,&out);
        mod.initialize();
        BOOST_REQUIRE_NO_THROW(mod.process());

        // length check
        BOOST_REQUIRE(out.hasData());
        DataSet< uint8_t >* oSet = NULL;
        out.getReadData(oSet);
        BOOST_CHECK(oSet->data.size() == num / nuarr[qi]); // good size

        // load data from output file
        DataSet< uint8_t >* oSet2 = NULL;
        int num2 = RawFileUtility::getNumElements<uint8_t>("output" + qamarrs[qi]
          + hyerarrs[hi]);
        out2.getWriteData(oSet2, num2);
        RawFileUtility::read(oSet2->data.begin(), oSet2->data.end(), "output"
          + qamarrs[qi] + hyerarrs[hi]);
        BOOST_CHECK(oSet2->data.size() == oSet->data.size()); // good size

        // value check
        for(int i=0;i<oSet2->data.size();i++)
        {
            BOOST_CHECK(oSet->data[i] == oSet2->data[i]);
        }
        out2.releaseWriteData(oSet2);
        out.releaseReadData(oSet);
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
