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
 * @file KernelPoints.hh 
 * @brief The pairs of x,y index values for points in the cloud, and outside
 *        the cloud, for a kernel
 * @class KernelPoints
 * @brief The pairs of x,y index values for points in the cloud, and outside
 *        the cloud, for a kernel
 */

#ifndef KERNEL_POINTS_H
#define KERNEL_POINTS_H

#include <euclid/PointList.hh>

class MdvxProj;
class Grid2d;
class RepohParams;

//*------------------------------------------------------------------
class KernelPoints
{
public:
  /**
   * Empty
   */
  KernelPoints(void);

  /**
   * Set using inputs
   *
   * @param[in] mask          mask for clump points, for this kernel
   * @param[in] outsideMask   mask for all points not in a clump
   * @param[in] proj          projection
   * @param[in] centerPt      kernel centerpoint
   * @param[in] atRadar       true if kernel is at radar
   * @param[in] isFar         True for kernel farther from radar within a pair
   * @param[in] params        parameters
   */
  KernelPoints(const Grid2d &mask, const Grid2d &outsideMask,
	       const MdvxProj &proj, const Point &centerPt,
	       bool atRadar, bool isFar, const RepohParams &params);

  /**
   * Destructor
   */
  ~KernelPoints(void);


  /**
   * @return true if this object has enough points and is set
   */
  inline bool isBigEnough(void) const {return _bigEnough;}
  
  /**
   * @return reference to the cloud points
   */
  inline PointList &cloudPts(void) {return _pts;}

  /**
   * @return reference to the outside of cloud points
   */
  inline PointList &outPts(void) {return _outPts;}

  /**
   * @return reference to the cloud points
   */
  inline const PointList &cloudPts(void) const {return _pts;}

  /**
   * @return reference to the outside of cloud points
   */
  inline const PointList &outPts(void) const {return _outPts;}

  /**
   * @return number of points in the cloud
   */
  inline int npt(void) const {return (int)_pts.size();}

  /**
   * @return number of points just outside the cloud
   */
  inline int nptOutside(void) const {return (int)_outPts.size();}

  /**
   * Add a point to the cloud points
   *
   * @param[in] xy 
   */
  inline void add(const Point &xy)
  {
    _pts.append(xy);
  }

  /**
   * Add a point to the 'outside cloud' points
   *
   * @param[in] xy
   */
  inline void addOutside(const Point &xy)
  {
    _outPts.append(xy);
  }

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * @return Debug string showing content
   * @param[in] kmPerGate
   */
  std::string sprintDebug(double kmPerGate) const;

  /**
   * print cloude data values to a file
   * @param[in,out]  fp   File pointer
   * @param[in]  g  Grid data
   */
  void printData(FILE *fp, const Grid2d &g) const;

  /**
   * write value to a grid at the kernel points
   * @param[in,out] gr  Grid
   * @param[in] v  Value
   * @param[in] outside  True to use the 'outside' points, false to use cloud
   */
  void toGrid(Grid2d &gr, double v,  bool outside) const;

  /**
   * At the kernel points, get value from grid, convert to linear units,
   * take average of linear values, then convert back to log scale and return
   * that value
   *
   * @param[in] grid  Grid
   * @param[in] outside  True to use the 'outside' points, false to use cloud
   *
   * @return the final value
   */
  double meanLinearAdjusted(const Grid2d &grid, bool outside) const;

  /**
   * At the kernel points, get value from grid, convert to linear units,
   * take average of linear values, and return the linear value
   *
   * @param[in] grid  Grid
   * @param[in] outside  True to use the 'outside' points, false to use cloud
   *
   * @return the final value
   */
  double meanLinear(const Grid2d &grid, bool outside) const;

  
  /**
   * At the kernel points, get all values from grid, and return the
   * (max - min) 
   *
   * @param[in] grid  Grid
   * @param[in] outside  True to use the 'outside' points, false to use cloud
   *
   * @return max - min
   */
  double dataRange(const Grid2d &grid, bool outside) const;

  /**
   * @return the average X value for the points in the cloud
   */
  inline double xAverage(void) const {return _pts.xAverage();}

  /**
   * Remove points from cloud until the data values within the cloud do not
   * have outliers, or until a small number of points remain
   * @param[in] data  Data to look in in clude
   * @param[in] diffThresh  Max allowed difference beteween min and max values
   * @param[in] minSize  Minimum number of cloud points
   */
  void removeOutliers(const Grid2d &data, double diffThresh, int minSize);

  /**
   * @return true if at least this many points in the cloud
   * @param[in]  minKernelSize 
   */
  bool enoughPoints(int minKernelSize) const;

  
protected:
private:

  bool _bigEnough;    /**< True if points is big enough */
  PointList _pts;     /**< Cloud points */
  PointList _outPts;  /**< Outside points */
};

#endif
 
