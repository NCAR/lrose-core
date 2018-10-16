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
 * @file KernelGrids.cc
 * @brief KernelGrids main class
 */
#include "KernelGrids.hh"

/*----------------------------------------------------------------*/
KernelGrids::KernelGrids() : MathUserData()
{
  _sdbz = NULL;
  _kdbzUnadjusted = NULL;
  _szdr = NULL;
  _pid = NULL;
  _snoise = NULL;
  _knoise = NULL;
  _srhohv = NULL;
  _kdbzAdjusted = NULL;
  _dbz_diff = NULL;
}

/*----------------------------------------------------------------*/
KernelGrids::KernelGrids(const Grid2d **sdbz, const Grid2d **kdbz,
			 const Grid2d **szdr, const Grid2d **pid,
			 const Grid2d **snoise, const Grid2d **knoise,
			 const Grid2d **srhohv, const Grid2d **kdbzAdjusted,
			 const Grid2d **dbzDiff)
  : MathUserData()
{
  _sdbz = *sdbz;
  _kdbzUnadjusted = *kdbz;
  _szdr = *szdr;
  _pid = *pid;
  _snoise = *snoise;
  _knoise = *knoise;
  _srhohv = *srhohv;
  _kdbzAdjusted = *kdbzAdjusted;
  _dbz_diff = *dbzDiff;
}

/*----------------------------------------------------------------*/
KernelGrids::~KernelGrids()
{
}

/*----------------------------------------------------------------*/
bool KernelGrids::getFloat(double &f) const
{
  return false;
}
