/**
 * \file components/gpp/phy/Dvbt1Puncturer/test/Dvbt1PuncturerComponent_test.cpp
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
 * Main test file for Dvbt1ConvInterleaver component.
 */

#define BOOST_TEST_MODULE Dvbt1PuncturerComponent_test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>

#include "../Dvbt1PuncturerComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;

BOOST_AUTO_TEST_SUITE (Dvbt1PuncturerComponent_Test)

BOOST_AUTO_TEST_CASE(Dvbt1PuncturerComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(Dvbt1PuncturerComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(Dvbt1PuncturerComponent_Parm_Test)
{
  Dvbt1PuncturerComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("coderate") == "34");
}

BOOST_AUTO_TEST_CASE(Dvbt1PuncturerComponent_Ports_Test)
{
  Dvbt1PuncturerComponent mod("test");
  BOOST_REQUIRE_NO_THROW(mod.registerPorts());

  vector<Port> iPorts = mod.getInputPorts();
  BOOST_REQUIRE(iPorts.size() == 1);
  BOOST_REQUIRE(iPorts.front().portName == "input1");
  BOOST_REQUIRE(iPorts.front().supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  vector<Port> oPorts = mod.getOutputPorts();
  BOOST_REQUIRE(oPorts.size() == 1);
  BOOST_REQUIRE(oPorts.front().portName == "output1");
  BOOST_REQUIRE(oPorts.front().supportedTypes.front() ==
      TypeInfo< uint8_t >::identifier);

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);
  BOOST_REQUIRE(oTypes["output1"] == TypeInfo< uint8_t >::identifier);
}

BOOST_AUTO_TEST_CASE(Dvbt1PuncturerComponent_Init_Test)
{
  Dvbt1PuncturerComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< uint8_t >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(Dvbt1PuncturerComponent_Process_Test)
{
  int grossnum = 10000000;
  
  // generate random data
  int codearr[] = {12, 23, 34, 56, 78};
  int codein[] =  { 2,  4,  6, 10, 14};
  int codeout[] = { 2,  3,  4,  6,  8};
  string codenamearr[] = {"1/2","2/3","3/4","5/6","7/8"};
  string codenamearrsh[] = {"12","23","34","56","78"};

  for(int cri = 0; cri < sizeof(codearr) / sizeof(codearr[0]); cri++)
  {
    if(true)
    {
      Dvbt1PuncturerComponent mod("test");
      mod.setValue("coderate", codearr[cri]);
      mod.registerPorts();

      map<string, int> iTypes,oTypes;
      iTypes["input1"] = TypeInfo< uint8_t >::identifier;
      mod.calculateOutputTypes(iTypes,oTypes);

      DataBufferTrivial<uint8_t> in;
      DataBufferTrivial<uint8_t> out;

      // Create enough data
      DataSet< uint8_t >* iSet = NULL;
      int num = 420*((2*8*204*(grossnum/204))/420); // bits (LCM of codein)
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
      BOOST_CHECK(oSet->data.size() == (codeout[cri] * num) / codein[cri]); // good size
      out.releaseReadData(oSet);
    }

    // load MATLAB/OCTAVE files
    if(true)
    {
      Dvbt1PuncturerComponent mod("test");
      mod.setValue("coderate", codearr[cri]);
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
      BOOST_CHECK(oSet->data.size() == (codeout[cri] * num) / codein[cri]); // good size

      // load data from output file
      DataSet< uint8_t >* oSet2 = NULL;
      int num2 = RawFileUtility::getNumElements<uint8_t>("output" + codenamearrsh[cri]);
      out2.getWriteData(oSet2, num2);
      RawFileUtility::read(oSet2->data.begin(), oSet2->data.end(), "output" + codenamearrsh[cri]);
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

BOOST_AUTO_TEST_SUITE_END()
