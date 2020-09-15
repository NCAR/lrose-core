// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// SimScanStrategy.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// SimScanStrategy reads IWRF data from specified files, converts
// the data to APAR TS format, and writes the
// converted files to a specified location
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <toolsa/str.h>

#include "SimScanStrategy.hh"
#include "WriteToFile.hh"
#include "WriteToUdp.hh"
#include "WriteToFmq.hh"
#include "ReadFromUdp.hh"

using namespace std;

// Constructor

SimScanStrategy::SimScanStrategy(const Params &params) :
        _params(params)
  
{

  _init();

}

// destructor

SimScanStrategy::~SimScanStrategy()

{

}

//////////////////////////////////////////////////
// get the next entry

SimScanStrategy::angle_t SimScanStrategy::getNextAngle() const
{
  
  if (_angleIndex >= _angles.size()) {
    _angleIndex = 0;
    _simVolNum++;
  }

  angle_t angle = _angles[_angleIndex];
  angle.volNum = _simVolNum;
  _angleIndex++;
  return angle;

}

////////////////////////////////////////////////////////////////////////
// initialize - compute simulated scan strategy

void SimScanStrategy::_init()

{
  
  _angles.clear();
  _simVolNum = 0;
  _angleIndex = 0;
  
  for (int isweep = 0; isweep < _params.sim_sweeps_n; isweep++) {
    
    const Params::sim_sweep_t &sweep = _params._sim_sweeps[isweep];
    
    if (sweep.sweep_type == Params::RHI_SIM) {
      
      for (double az = sweep.min_az;
           az <= sweep.max_az;
           az += sweep.delta_az) {

        for (int istride = 0; istride < _params.n_beams_per_dwell; istride++) {

          for (double el = sweep.min_el + istride * sweep.delta_el;
               el <= sweep.max_el;
               el += _params.n_beams_per_dwell * sweep.delta_el) {

            angle_t angle;
            angle.el = el;
            angle.az = az;
            angle.sweepNum = isweep;
            angle.volNum = 0;
            angle.sweepMode = Radx::SWEEP_MODE_RHI;
            _angles.push_back(angle);

          } // el

        } // istride

      } // az

    } else if (sweep.sweep_type == Params::PPI_SIM) {

      for (double el = sweep.min_el;
           el <= sweep.max_el;
           el += sweep.delta_el) {

        for (int istride = 0; istride < _params.n_beams_per_dwell; istride++) {
          
          for (double az = sweep.min_az + istride * sweep.delta_az;
               az <= sweep.max_az;
               az += _params.n_beams_per_dwell * sweep.delta_az) {

            angle_t angle;
            angle.el = el;
            angle.az = az;
            angle.sweepNum = isweep;
            angle.volNum = 0;
            angle.sweepMode = Radx::SWEEP_MODE_SECTOR;
            _angles.push_back(angle);

          } // az

        } // istride

      } // el
      
    } // if (sweep.sweep_type == Params::RHI_SIM)
    
  } // isweep

}

