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
 * @file Kernel.hh 
 * @brief kernel around an interesting point, along a beam
 *
 * @class Kernel
 * @brief kernel around an interesting point, along a beam
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "RepohParams.hh"
#include "KernelData.hh"
#include "KernelPoints.hh"
#include <euclid/PointList.hh>
#include <string>
#include <vector>

class MdvxProj;
class Grid2d;
class CloudGap;
class DsSpdb;
class KernelGrids;

//*------------------------------------------------------------------
class Kernel
{
public:

  /**
   * @param[in] vlevel        Vertical level degrees
   * @param[in] isFar         True for kernel farther from radar
   * @param[in] mask          mask for clump points, for this kernel
   * @param[in] outsideMask   mask for all points not in a clump
   * @param[in] g             The cloud gap along the beam
   * @param[in] params        parameters
   * @param[in] gp            projection
   *
   * For 'far' kernels, we look at a near edge of a cloud, for 'near' kernels,
   * opposite
   */
  Kernel(double vlevel, bool isFar, const Grid2d &mask,
	 const Grid2d &omask, const CloudGap &g, 
	 const RepohParams &params, const MdvxProj &gp);

  ~Kernel(void);

  /**
   * Put centerpoint to a grid with value = local _color
   * @param[in,out] g  
   */
  void centerpointToGrid(Grid2d &g) const;

  /**
   * Print out a one line status summary to stdout
   */
  void printStatus(void) const;

  /**
   * Print out all the kernel points
   */
  void print(void) const;

  /**
   * Debugging: Print out kernel contents to a file for use by Scott Ellis
   * in a format he likes.
   *
   * @param[in] dir  Path to write to
   * @param[in] id   ID value
   * @param[in] vlevel  Vertical level angle
   * @param[in] grids  Data grids used for kernels 
   * @param[in]  kmPerGate  Kilometer gate spacing
   */
  void printDebug(const std::string dir, int id, double vlevel,
		  const KernelGrids &grids, double kmPerGate);

  /**
   * Finish processing a kernel of points using inputs.
   *
   * @param[in] time    Data time
   * @param[in] id      id to give kernel
   * @param[in] vlevel  degrees
   * @param[in] grids   grids used for kernels
   * @param[in] P       params
   * @param[in] kmPerGate  Kilometer gate spacing
   *
   * Decide if its good or bad using equations
   *
   * Print summary status
   */
  void finishProcessing(const time_t &time, int id, double vlevel,
			const KernelGrids &grids, const RepohParams &P,
			double kmPerGate);

  /**
   * Write a genpoly surrounding the kernel points to Spdb,
   * outside = true for points just outside the kernel
   *
   * does not write when at origin
   *
   * @param[in] time    Data time
   * @param[in] outside  Flag telling which points to use
   * @param[in] proj  Projection
   * @param[in,out] D  Spdb server object
   */
  bool writeGenpoly(const time_t &t, bool outside, const MdvxProj &proj,
		    DsSpdb &D) const;

  /**
   * return the average point x and y, and the total attenuation
   * @param[out] x  Average x index for kernel points
   * @param[out] y  Beam index for this object 
   * @param[out] atten  Total Attenuation
   */
  void getAttenuation(double &x, double &y, double &atten) const;

  /**
   * cubic equation for humidity using attenuation
   * @param[in] a  Attenuation
   * @return Humidity
   */
  static double humidityFromAttenuation(const double a);

  /**
   * @return true if the kernel is one at the radar
   */
  inline bool atRadar(void) const
  {
    return _center.getX() == 0;
  }

  /**
   * @return true if kernel data has passed all the tests
   */
  inline bool passedTests(void) const {return _data.passedTests();}

  /**
   * @return true if kernel has reasonable number of points
   */
  inline bool isBigEnough(void) const {return _kpts.isBigEnough();}
  
protected:
private:

  int _color;              /**< Color index for points */
  double _vlevel;          /**< Vertical level degrees */
  Point _center;           /**< Kernel center point */

  KernelPoints _kpts;      /**< Kernel points, filtered */
  KernelPoints _totalKpts; /**< Kernel points, all */

  KernelData _data;        /**< Data values and flags for this kernel */


  double _Ag;              /**< Attenuation */
  double _q;               /**< Humidiity */


  double _gaseousAttenuation(const double km_per_gate) const;
  bool _initializeDebugging(const MdvxProj &gp, double vlevel, 
			    const RepohParams &params) const;
  bool _isDebug(const RepohParams::mask_t &mask, double range,
		double az, double vlevel) const;
};

#endif
 
