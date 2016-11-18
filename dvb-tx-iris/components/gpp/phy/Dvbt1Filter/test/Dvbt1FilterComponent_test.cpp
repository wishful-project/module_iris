/**
 * \file components/gpp/phy/Dvbt1Filter/test/Dvbt1FilterComponent_test.cpp
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
 * Main test file for Dvbt1Filter component.
 */

#define BOOST_TEST_MODULE Dvbt1FilterComponent_test

#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "../Dvbt1FilterComponent.h"
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
using namespace boost::lambda;
namespace bp = boost::posix_time;

BOOST_AUTO_TEST_SUITE (Dvbt1FilterComponent_Test)

BOOST_AUTO_TEST_CASE(Dvbt1FilterComponent_Basic_Test)
{
  BOOST_REQUIRE_NO_THROW(Dvbt1FilterComponent mod("test"));
}

BOOST_AUTO_TEST_CASE(Dvbt1FilterComponent_Parm_Test)
{
  Dvbt1FilterComponent mod("test");
  BOOST_CHECK(mod.getParameterDefaultValue("debug") == "false");
  BOOST_CHECK(mod.getParameterDefaultValue("samplerate") == "0.0");
  BOOST_CHECK(mod.getParameterDefaultValue("stopband") == "4000000.0");
  BOOST_CHECK(mod.getParameterDefaultValue("attenuation") == "35.0");
  BOOST_CHECK(mod.getParameterDefaultValue("coeffsfile") == "");
}

BOOST_AUTO_TEST_CASE(Dvbt1FilterComponent_Ports_Test)
{
  typedef std::complex<float>   Cplx;
  
  Dvbt1FilterComponent mod("test");
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

BOOST_AUTO_TEST_CASE(Dvbt1FilterComponentComponent_Init_Test)
{
  typedef std::complex<float>   Cplx;

  Dvbt1FilterComponent mod("test");
  mod.registerPorts();

  map<string, int> iTypes,oTypes;
  iTypes["input1"] = TypeInfo< Cplx >::identifier;
  mod.calculateOutputTypes(iTypes,oTypes);

  BOOST_REQUIRE_NO_THROW(mod.initialize());
}

BOOST_AUTO_TEST_CASE(Dvbt1FilterComponent_Process_Test)
{
  
  double outarr[] = {100e6/10,100e6/9,100e6/8,100e6/7};
  double sbarr[] = {3950000,4000000,4050000};
  double atarr[] = {25,30,35};

  typedef std::complex<float>   Cplx;

  for(int oi = 0; oi < sizeof(outarr) / sizeof(outarr[0]); oi++)
  {
    for(int si = 0; si < sizeof(sbarr) / sizeof(sbarr[0]); si++)
    {
      for(int ai = 0; ai < sizeof(atarr) / sizeof(atarr[0]); ai++)
      {
        // test passband
        if(true)
        {
          Dvbt1FilterComponent mod("test");
          mod.setValue("samplerate", outarr[oi]);
          mod.setValue("attenuation", atarr[ai]);
          mod.setValue("stopband", sbarr[si]);
          mod.registerPorts();

          map<string, int> iTypes,oTypes;
          iTypes["input1"] = TypeInfo< Cplx >::identifier;
          mod.calculateOutputTypes(iTypes,oTypes);

          DataBufferTrivial< Cplx > in;
          DataBufferTrivial< Cplx > out;

          DataSet< Cplx >* iSet = NULL;
          int num = 100000; // samples
          double dtheta = 2 * M_PI * (64.0e6/7.0) * (0.5*6817/8192) / outarr[oi];
          in.getWriteData(iSet, num);
          for(int i=0;i<num;i++)
              iSet->data[i] = Cplx(1*cos(dtheta * i), 0); // cosine in passband
              
          // input power
          float powin = 0;
          for(int i=0;i<num;i++)
            powin += iSet->data[i].real() * iSet->data[i].real() 
              + iSet->data[i].imag() * iSet->data[i].imag();
          powin /= num;

          mod.setBuffers(&in,&out);
          mod.initialize();
          BOOST_REQUIRE_NO_THROW(mod.process());
            
          // length check
          BOOST_REQUIRE(out.hasData());
          DataSet< Cplx >* oSet = NULL;
          out.getReadData(oSet);
          printf("iSet->data.size()=%lu, oSet->data.size()=%lu\n", iSet->data.size(), oSet->data.size());
          BOOST_CHECK(oSet->data.size() == iSet->data.size()); // same size
          
          // output power
          float powout = 0;
          int trans = 1000;
          for(int i=trans;i<num;i++)
            powout += oSet->data[i].real() * oSet->data[i].real() 
              + oSet->data[i].imag() * oSet->data[i].imag();
          powout /= (num-trans);
          
          // check attenuation (+/-0.5 dB)
          float attdb = 10 * log10(powin/powout);
          printf("powin = %f, powout = %f, att = %.1f dB\n", powin, powout, attdb);
          BOOST_CHECK(fabs(attdb - 0) < 0.5);
         
          in.releaseWriteData(iSet);
          out.releaseReadData(oSet);
        }
        
        // test stopband
        if(true)
        {
          Dvbt1FilterComponent mod("test");
          mod.setValue("samplerate", outarr[oi]);
          mod.setValue("attenuation", atarr[ai]);
          mod.setValue("stopband", sbarr[si]);
          mod.registerPorts();

          map<string, int> iTypes,oTypes;
          iTypes["input1"] = TypeInfo< Cplx >::identifier;
          mod.calculateOutputTypes(iTypes,oTypes);

          DataBufferTrivial< Cplx > in;
          DataBufferTrivial< Cplx > out;

          DataSet< Cplx >* iSet = NULL;
          int num = 1000000; // samples
          double dtheta = 2 * M_PI * (sbarr[si]+80e3) / outarr[oi];
          in.getWriteData(iSet, num);
          for(int i=0;i<num;i++)
              iSet->data[i] = Cplx(1*cos(dtheta * i), 0); // cosine in stopband
              
          // input power
          float powin = 0;
          for(int i=0;i<num;i++)
            powin += iSet->data[i].real() * iSet->data[i].real() 
              + iSet->data[i].imag() * iSet->data[i].imag();
          powin /= num;

          mod.setBuffers(&in,&out);
          mod.initialize();
          BOOST_REQUIRE_NO_THROW(mod.process());
            
          // length check
          BOOST_REQUIRE(out.hasData());
          DataSet< Cplx >* oSet = NULL;
          out.getReadData(oSet);
          printf("iSet->data.size()=%lu, oSet->data.size()=%lu\n", iSet->data.size(), oSet->data.size());
          BOOST_CHECK(oSet->data.size() == iSet->data.size()); // same size
          
          // output power
          float powout = 0;
          float maxout = 0;
          int trans = 1000;
          for(int i=trans;i<num-trans;i++)
          {
            powout += oSet->data[i].real() * oSet->data[i].real() 
              + oSet->data[i].imag() * oSet->data[i].imag();
            if(oSet->data[i].real()>maxout)
              maxout = oSet->data[i].real();
          }
          powout /= (num-2*trans);
          
          // check attenuation (+/-8 dB)
          float attdb = 10 * log10(powin/powout);
          printf("powin = %f, powout = %f, maxout = %f, ATT = %.1f, att = %.1f dB\n", powin, powout, maxout, atarr[ai], attdb);
          BOOST_CHECK(attdb > atarr[ai] - 0.5);
         
          in.releaseWriteData(iSet);
          out.releaseReadData(oSet);
        }
      }
    }
  }

}

BOOST_AUTO_TEST_SUITE_END()
