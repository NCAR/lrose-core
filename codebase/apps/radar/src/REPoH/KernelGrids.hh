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
 * @file KernelGrids.hh 
 * @brief KernelGrids placeholders pointing to grids used in Kernel calulations
 * @class KernelGrids
 * @brief KernelGrids placeholders pointing to grids used in Kernel calulations
 */

#ifndef KERNELGRIDS_H
#define KERNELGRIDS_H
#include <euclid/Grid2d.hh>
#include <rapmath/MathUserData.hh>
class Grid2d;

//------------------------------------------------------------------
class KernelGrids : MathUserData
{
public:
  KernelGrids(void);
  KernelGrids(const Grid2d **sdbz, const Grid2d **kdbz,
	      const Grid2d **szdr, const Grid2d **pid,
	      const Grid2d **snoise, const Grid2d **knoise,
	      const Grid2d **srhohv, const Grid2d **kdbzAdjusted,
	      const Grid2d **dbzDiff);
  ~KernelGrids(void);
  #include <rapmath/MathUserDataVirtualMethods.hh>


  const Grid2d *_sdbz, *_kdbzUnadjusted;
  const Grid2d *_szdr, *_pid, *_snoise, *_knoise, *_srhohv;
  const Grid2d *_dbz_diff;
  const Grid2d *_kdbzAdjusted;

protected:
private:

};

#endif
 
