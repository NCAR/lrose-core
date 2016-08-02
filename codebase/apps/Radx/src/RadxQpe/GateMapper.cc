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
/**
 * @file GateMapper.cc
 */

#include "GateMapper.hh"
#include "Geom.hh"
#include <toolsa/LogMsg.hh>

//----------------------------------------------------------------
GateMapper::GateMapper(void) : _isOK(false)
{
}


//----------------------------------------------------------------
GateMapper::GateMapper(double radx_r0, int radx_nr, double radx_dr,
		       const Geom &geom) : _isOK(true)
{  
  double grid_r0 = geom.r0();
  double grid_dr = geom.dr();
  int grid_nr = geom.nGate();
  if (grid_dr != radx_dr)
  {
    LOGF(LogMsg::WARNING, "Uneven gate spacing grid2d:%lf  radx:%lf",
	 grid_dr, radx_dr);
    _isOK = false;
    return;
  }
  double minr, maxr;
  if (radx_r0 >= grid_r0)
  {
    minr = radx_r0;
  }
  else
  {
    minr = grid_r0;
  }

  if (radx_r0 + radx_nr*grid_dr >= grid_r0 + grid_nr*grid_dr)
  {
    maxr = grid_r0 + grid_nr*grid_dr;
  }
  else
  {
    maxr = radx_r0 + radx_nr*grid_dr;
  }

  for (double r=minr; r<= maxr; r += grid_dr)
  {
    int radxIndex = (int)((r-radx_r0)/grid_dr);
    int gateIndex = (int)((r-grid_r0)/grid_dr);

    _map.push_back(GateMapper1(radxIndex, gateIndex));
  }
}

//----------------------------------------------------------------
GateMapper::~GateMapper(void)
{
}

