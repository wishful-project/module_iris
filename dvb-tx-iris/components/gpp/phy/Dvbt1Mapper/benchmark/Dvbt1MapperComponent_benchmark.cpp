/**
 * \file components/gpp/phy/Dvbt1Mapper/benchmark/Dvbt1MapperComponent_benchmark.cpp
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
 * Main benchmark file for Dvbt1Mapper component.
 */

#include "../Dvbt1MapperComponent.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include "utility/DataBufferTrivial.h"
#include "utility/RawFileUtility.h"

using namespace std;
using namespace iris;
using namespace iris::phy;
namespace bp = boost::posix_time;

int main(int argc, char* argv[])
{
  int qamarr[] = {4,16,64};
  int qamlogarr[] = {2,4,6};
  int hyerarr[] = {0,1,2,4};
  typedef std::complex<float>   Cplx;

  for(int hi = 0; hi < sizeof(hyerarr) / sizeof(hyerarr[0]); hi++)
  {
    for(int qi = 0; qi < sizeof(qamarr) / sizeof(qamarr[0]); qi++)
    {
      Dvbt1MapperComponent mod("test");
      mod.setValue("qammapping", qamarr[qi]);
      mod.setValue("hyerarchymode", hyerarr[hi]);
      mod.registerPorts();

      map<string, int> iTypes,oTypes;
      iTypes["input1"] = TypeInfo< uint8_t >::identifier;
      mod.calculateOutputTypes(iTypes,oTypes);

      DataBufferTrivial< uint8_t > in;
      DataBufferTrivial< Cplx > out;

      DataSet< uint8_t >* iSet = NULL;
      int num = (3*(8*204*(10000000/204))/4)/(log(qamarr[qi])/log(2)); // symbols
      in.getWriteData(iSet, num);
      boost::random::mt19937 rng;
      boost::random::uniform_int_distribution<> uptosix(0,qamlogarr[qi]-1); 
      for(int i=0;i<num;i++)
          iSet->data[i] = uint8_t(uptosix(rng)); // random data
      in.releaseWriteData(iSet);

      mod.setBuffers(&in,&out);
      mod.initialize();

      bp::ptime t1(bp::microsec_clock::local_time());
      mod.process();
      bp::ptime t2(bp::microsec_clock::local_time());
      bp::time_duration time = t2-t1;

      float symsPerSec = ((num)/(1.0e-9 * time.total_nanoseconds()));
      cout << qamarr[qi] << "-QAM, alpha = " << hyerarr[hi] << ": Rate = "
        << symsPerSec*1.0e-6 << " Msym/s" << endl;

      float maxRate = (64.0e6/7) * (1.0/(1+1/32.0)) * (6048.0/8192.0); // table 4 pag. 26
      cout << "Maximum interleaved symbol rate in ETSI EN 300 744 is "
        << maxRate*1.0e-6 << " Msym/s" << endl;

      if(symsPerSec > 1.2 * maxRate)
        cout << "    >>>> " << symsPerSec/maxRate << "x: GOOD! <<<<" << endl;
      else
        cout << "    >>>> " << symsPerSec/maxRate << "x: OOPS! <<<<" << endl;
    }
  }
}

