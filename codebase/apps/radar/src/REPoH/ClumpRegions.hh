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
 * @file ClumpRegions.hh
 * @brief 
 * @class ClumpRegions
 * @brief 
 *
 * ClumpRegions produces connected subsets of points called 'regions'.
 */

# ifndef    ClumpRegions_HH
# define    ClumpRegions_HH

#include "ClumpRegion.hh"
#include <rapmath/MathUserData.hh>
#include <euclid/PointList.hh>
#include <vector>

class ClumpRegions : public MathUserData
{
public:

  /**
   * Build the clumps from the input data and store results as local
   * ClumpRegion vector
   *
   * @param[in] data
   */
  ClumpRegions(const Grid2d &data);

  virtual ~ClumpRegions(void);

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return number of regions
   */
  inline size_t size(void) const {return _regions.size();}

  /**
   * @return reference to a region
   * @param[in] i  Index
   */
  inline const PointList & operator[](size_t i) const {return _regions[i];}

  /**
   * @return reference to a region
   * @param[in] i  Index
   */
  inline PointList & operator[](size_t i) {return _regions[i];}


  /**
   * Set values for index'th region to a particular color in a grid
   * @param[in] index  ClumpRegion index
   * @param[in] color  Value to set
   * @param[in,out] out  Grid to modify
   */
  void setValues(int index, double color, Grid2d &out) const;

  
  /**
   * Set values for index'th region to missing in a grid if the region
   * is too small
   * @param[in] index  ClumpRegion index
   * @param[in] minPt  Minimum number of points to keep it
   * @param[in,out] out  Grid to modify
   */
  void removeSmallClump(int index, int minPt, Grid2d &out) const;


private:
  
  std::vector<PointList> _regions;
};  

#endif
