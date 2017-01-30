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
/////////////////////////////////////////////////////////////
// FilterUtils.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////

/**
 * @file FilterUtils.hh
 * @class FilterUtils
 * @brief Filter utilities for radar data
 */

#ifndef FilterUtils_HH
#define FilterUtils_HH

using namespace std;

////////////////////////
// This class

class FilterUtils {
  
public:

  /**
   * Apply a median filter to an array of double values
   * @param[in][out] field pointer to the data
   * @param[in] fieldLen - length of the field array
   * @param[in] filterLen - length of the median filter - should be odd
   */
  static void applyMedianFilter(double *field,
				int fieldLen,
				int filterLen);
  
  /**
   * Apply a median filter to an array of double values
   *   Handles missing data.
   * @param[in][out] field pointer to the data
   * @param[in] fieldLen - length of the field array
   * @param[in] filterLen - length of the median filter - should be odd
   * @param[in] missingVal - value to indicate missing data
   */

  static void applyMedianFilter(double *field,
                                int fieldLen,
                                int filterLen,
                                double missingVal);
  
  /**
   * Apply a median filter to an array of integer values
   * @param[in][out] field Pointer to the data
   * @param[in] fieldLen The length of the field array
   * @param[in] filterLen The length of the median filter. Must be an odd number!
   */
  static void applyMedianFilter(int *field,
				int fieldLen,
				int filterLen);
  
  /**
   * Apply a median filter to an array of int values
   *   Handles missing data.
   * @param[in][out] field pointer to the data
   * @param[in] fieldLen - length of the field array
   * @param[in] filterLen - length of the median filter - should be odd
   * @param[in] missingVal - value to indicate missing data
   */

  static void applyMedianFilter(int *field,
                                int fieldLen,
                                int filterLen,
                                int missingVal);
  
  /**
   * Interpolate linearly between points
   * @param[in] xx1 The x1 coordinate of the line to interpolate
   * @param[in] xx2 The x2 coordinate of the line to interpolate
   * @param[in] yy1 The y1 coordinate of the line to interpolate
   * @param[in] yy2 The y2 coordinate of the line to interpolate
   * @param[in] xVal The x value to find a y value for
   * @return The interpolated yVal for the given xVal
   */
  static double linearInterp(double xx1, double yy1,
			     double xx2, double yy2,
			     double xVal);

  /**
   * Compute standard deviation of a field, over a kernel in range
   * Set field values to missingVal if they are missing.
   * The sdev will be set to missingVal if not enough data is
   * available for computing the standard deviation.
   * @param[in] field The data to compute a standard deviation for
   * @param[out] sdev The computed sdev array
   * @param[in] nGates Number of gates in the input array
   * @param[in] nGatesKernel Number of gates over which to compute sdev
   * @param[in] missingVal The value to use for missing data
   */
  static void computeSdevInRange(const double *field,
				 double *sdev,
				 int nGates,
				 int nGatesKernel,
				 double missingVal);
  
  // compute trend deviation of a field, over a kernel in range
  //
  // Set field values to missingVal if they are missing.
  // The trend dev will be set to missingVal if not enough data is
  // available for computing the result.
  //
  // If mean is not NULL, the mean values are stored in that array.
  // If texture is not NULL, the texture values are stored there.
  //
  // The trend deviation differs from standard deviation.
  // It is computed as follows:
  //  (a) compute kernel mean at each point
  //  (b) compute residual of data from trend mean
  //  (c) compute root mean square of residual over kernel.
  
  static void computeTrendDevInRange(const double *field,
                                     int nGates,
                                     int nGatesKernel,
                                     double missingVal,
                                     double *mean = NULL,
                                     double *texture = NULL);
  
protected:
private:

  /**
   * Comparison function needed for double qsort
   * @param[in] i Pointer to the first value to compare
   * @param[in] j Pointer to the second value to compare
   * @return -1 if i<j, 0 if i==j, 1 if i>j
   */
  static int _doubleCompare(const void *i, const void *j);

  /**
   * Comparison function needed for integer qsort
   * @param[in] i Pointer to the first value to compare
   * @param[in] j Pointer to the second value to compare
   * @return -1 if i<j, 0 if i==j, 1 if i>j
   */
  static int _intCompare(const void *i, const void *j);

};

#endif

