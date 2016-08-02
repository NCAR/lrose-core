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
 * @file Kernels.hh 
 * @brief Kernels The set of Kernels
 * @class Kernels
 * @brief Kernels The set of Kernels
 * 
 */

#ifndef Kernels_H
#define Kernels_H

#include "KernelPair.hh"
#include <vector>

/*----------------------------------------------------------------*/
class Kernels
{
public:
  Kernels();
  ~Kernels();

  inline void append(const KernelPair &k) {_k.push_back(k);}

  void print(void) const;

  /**
   * Return # of Kernel pairs
   */
  inline int num(void) const
  {
    return (int)_k.size();
  }

  /**
   * Return ref. to the 'ith' Kernel pair
   */
  inline const KernelPair &ith_kernel_pair(const int i) const
  {
    return _k[i];
  }

  /**
   * Finish up a KernelPair using the other inputs, then store locally
   */
  void add(const KernelPair &k, const time_t &time, const double vlevel, 
	   const KernelGrids &grids, const Params &params, const double dx,
	   Grid2d &kcp);


  /**
   * Make humidity estimation for all KernelPair objects, write results
   * to att and hum input grids, and return one line summary strings
   */
  string humidity_estimate(const double vlevel, const GridProj &gp,
			   Grid2d &att, Grid2d &hum) const;

  /**
   * Write genpoly representation of kernels to SPDB
   */
  bool write_genpoly(const string &url, const time_t &time, const int nx,
		     const int ny, const bool outside) const;

protected:
private:

  int _next_available_id;
  std::vector<KernelPair> _k;
};

#endif
 
