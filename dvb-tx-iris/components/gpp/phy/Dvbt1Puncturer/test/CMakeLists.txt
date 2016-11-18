#
# Copyright 2012-2016 The Iris Project Developers. See the
# COPYRIGHT file at the top-level directory of this distribution
# and at http://www.softwareradiosystems.com/iris/copyright.html.
#
# This file is part of the Iris Project.
#
# Iris is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# Iris is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# A copy of the GNU Lesser General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

########################################################################
# Build executable, register as test
########################################################################
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)
ADD_EXECUTABLE(Dvbt1PuncturerComponent_test Dvbt1PuncturerComponent_test.cpp)
TARGET_LINK_LIBRARIES(Dvbt1PuncturerComponent_test comp_gpp_phy_dvbt1puncturer_static ${Boost_LIBRARIES})
file(COPY input.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY output12.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY output23.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY output34.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY output56.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY output78.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
ADD_TEST(Dvbt1PuncturerComponent_test Dvbt1PuncturerComponent_test)
