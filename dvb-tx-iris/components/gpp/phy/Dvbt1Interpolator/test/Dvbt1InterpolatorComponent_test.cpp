/**
 * \file components/gpp/phy/Dvbt1OfdmMod/test/Dvbt1InterpolatorComponent_test.cpp
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
 * Main test file for Dvbt1Interpolator component.
 */

#define BOOST_TEST_MODULE Dvbt1InterpolatorComponent_test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>

#include "../Dvbt1InterpolatorComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;
namespace bp = boost::posix_time;

BOOST_AUTO_TEST_SUITE (Dvbt1InterpolatorComponent_Test)

BOOST_AUTO_TEST_CASE(Dvbt1InterpolatorComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(Dvbt1InterpolatorComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(Dvbt1InterpolatorComponent_Parm_Test)
{
  Dvbt1InterpolatorComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("insamplerate") == "0.0");
  BOOST_CHECK(mod.getParameterDefaultValue("outsamplerate") == "0.0");
  BOOST_CHECK(mod.getParameterDefaultValue("responsefile") == "");
}

BOOST_AUTO_TEST_CASE(Dvbt1InterpolatorComponent_Ports_Test)
{
  typedef std::complex<float>   Cplx;
  
  Dvbt1InterpolatorComponent mod("test");
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

BOOST_AUTO_TEST_CASE(Dvbt1InterpolatorComponent_Init_Test)
{
  typedef std::complex<float>   Cplx;

  Dvbt1InterpolatorComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(Dvbt1InterpolatorComponent_Process_Test)
{
  
  double outarr[] = {100e6/10,100e6/9,100e6/8,100e6/7};
  typedef std::complex<float>   Cplx;

  for(int oi = 0; oi < sizeof(outarr) / sizeof(outarr[0]); oi++)
  {
    // generate random data
    if(true)
    {
      Dvbt1InterpolatorComponent mod("test");
      mod.setValue("outsamplerate", outarr[oi]);
      mod.registerPorts();

      map<string, int> iTypes,oTypes;
      iTypes["input1"] = TypeInfo< Cplx >::identifier;
      mod.calculateOutputTypes(iTypes,oTypes);

      DataBufferTrivial< Cplx > in;
      DataBufferTrivial< Cplx > out;

      // Create enough data
      float maxRate = (64.0e6/7); // table 4 pag. 26
      DataSet< Cplx >* iSet = NULL;
      int num = 1000000;
      in.getWriteData(iSet, num);
      boost::random::mt19937 rng;
      boost::random::normal_distribution<> gauss(0,1);
      for(int i=0;i<num;i++)
          iSet->data[i] = Cplx(gauss(rng), gauss(rng)); // random complex data

      mod.setBuffers(&in,&out);
      mod.initialize();
      BOOST_REQUIRE_NO_THROW(mod.process());
  
      // length check
      BOOST_REQUIRE(out.hasData());
      DataSet< Cplx >* oSet = NULL;
      out.getReadData(oSet);
      printf("iSet->data.size()=%lu, oSet->data.size()=%lu\n", iSet->data.size(), oSet->data.size());
      BOOST_CHECK(oSet->data.size() > (iSet->data.size() * outarr[oi] / maxRate) - 1000); // good size
      in.releaseWriteData(iSet);
      out.releaseReadData(oSet);
    }

  }
}

BOOST_AUTO_TEST_SUITE_END()
