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
 * @file KernelPair.hh 
 * @brief KernelPair a pair of kernels along a beam between a gap
 * @class KernelPair 
 * @brief KernelPair a pair of kernels along a beam between a gap
 * 
 */

#ifndef KernelPair_H
#define KernelPair_H

#include "Kernel.hh"

class Grid2d;
class Params;
class GridProj;
class KernelGrids;
class DsSpdb;

/*----------------------------------------------------------------*/
class KernelPair
{
public:

  KernelPair (const double vlevel, const Grid2d &mask_far,
	      const Grid2d &mask_near, const Grid2d &omask,
	      const CloudGap &g, const Grid2d &clumps,
	      const Params &params, const GridProj &gp);
  ~KernelPair();

  /**
   * Ok means both kernels are well formed 
   */
  inline bool is_ok(void) const {return _near.is_ok() && _far.is_ok();}

  /**
   * good means both kernels pass all tests
   */
  inline bool is_good(void) const {return _near.is_good() && _far.is_good();}

  void print(void) const;

  /**
   * Write centerpoints to the input grid
   */
  void cp_to_grid(Grid2d &kcp) const;

  /**
   * Finish up both near and far kernel objects using inputs
   * give kernel objects:   id, id+1
   * Write to kcp input
   */
  void finish_processing(const time_t &time, const double vlevel,
			 const KernelGrids &grids, const Params &P,
			 const double km_per_gate, const int &id, Grid2d &kcp);

  /**
   * derive a humidity estimate for the gap between the two kernels.
   * write values to att and hum
   * return a one line string summary
   */
  string humidity_estimate(const double vlevel, const GridProj &gp,
			   Grid2d &att, Grid2d &hum) const;

  /**
   * Write kernel boundary as  genpoly to SPDB
   */
  bool write_genpoly(const time_t &t, const int nx, const int ny, 
		     const bool outside, DsSpdb &D) const;

protected:
private:

  Kernel _near;
  Kernel _far;
};

#endif
 
