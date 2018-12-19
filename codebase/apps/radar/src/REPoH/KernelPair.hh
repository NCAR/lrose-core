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
 * A near kernel, and a far kernel
 * 
 */

#ifndef KernelPair_H
#define KernelPair_H

#include "Kernel.hh"

class Grid2d;
class RepohParams;
class MdvxProj;
class KernelGrids;
class DsSpdb;

/*----------------------------------------------------------------*/
class KernelPair
{
public:

  /**
   * @param[in] vlevel        Vertical level degrees
   * @param[in] maskFar       mask for clump points, far kernel
   * @param[in] maskNear      mask for clump points, near kernel
   * @param[in] outsideMask   mask for all points not in a clump
   * @param[in] g             The cloud gap along the beam
   * @param[in] params        parameters
   * @param[in] gp            projection
   */
  KernelPair (double vlevel, const Grid2d &mask_far,
	      const Grid2d &mask_near, const Grid2d &outsideMask,
	      const CloudGap &g, const RepohParams &params,
	      const MdvxProj &gp);

  ~KernelPair();

  /**
   * @return true if both kernels have enough content
   */
  inline bool isBigEnough(void) const {return (_near.isBigEnough() &&
					       _far.isBigEnough());}

  /**
   * @return true if both kernels passed all the data related tests
   */
  inline bool passedTests(void) const
  {
    return _near.passedTests() && _far.passedTests();
  }

  /**
   * Debug print */
  void print(void) const;

  /**
   * Write near and far centerpoints to the grid
   * @param[in,out] kcp  The grid
   */
  void centerpointToGrid(Grid2d &kcp) const;

  /**
   * Finish up both near and far kernel objects using inputs
   * @param[in] time    Data time
   * @param[in] vlevel  degrees
   * @param[in] grids   grids used for kernels
   * @param[in] P       params
   * @param[in] kmPerGate  
   * @param[in] nearId  id to give near kernel
   * @param[in] farId   id to give far kernel
   */
  void finishProcessing(const time_t &time, double vlevel,
			const KernelGrids &grids, const RepohParams &P,
			double kmPerGate, int nearId, int farId);

  /**
   * Compute attenuation for one kernel pair and write value to the grid
   * @param[in] dx  Km per gate
   * @param[in,out] att  Attenuation output grid
   */
  void computeAttenuation(double dx, Grid2d &att) const;

  /**
   * Compute humidity for one kernel pair and write value to the grid
   * @param[in] dx  Km per gate
   * @param[in,out] hum  Humidity output grid
   */
  void computeHumidity(double dx, Grid2d &hum) const;

  /**
   * @return ascii output for this Kernel pair
   *
   * @param[in] vlevel vertical level degrees
   * @param[in] gp Grid projection
   */
  string asciiOutput(const double vlevel, const MdvxProj &gp) const;
  
  /**
   * @return the attenuation estimate for this pair
   *
   * @param[in] dx  Gate spacing km
   * @param[out] x0  Average x value for near kernel (gate index)
   * @param[out] x1  Average x value for far kernel (gate index)
   * @param[out] y0  Beam index
   */
  double attenuation(double dx, double &x0, double &x1, double &y0) const;

  /**
   * Write kernel boundary as  genpoly to SPDB
   * @param[in] t  data time
   * @param[in] outside true to write the kernel pair 'outside' points
   * @param[in] proj  grid projection
   * @param[in,out] D  Server object
   *
   * @return true for sucess
   */
  bool writeGenpoly(const time_t &t, bool outside,
		    const MdvxProj &proj, DsSpdb &D) const;

protected:
private:

  Kernel _near;    /**< Near kernel */
  Kernel _far;     /**< Far kernel */
};

#endif
 
