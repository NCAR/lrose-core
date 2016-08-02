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
 * @file GridExpand.hh 
 * @brief Expand a 2d grid in y dimension at lower and upper ends, replicating
 *        data assuming wraparound.  This is primarily to handle polar
 *        grids with full 360 degrees in y.
 * @class GridExpand
 * @brief Expand a 2d grid in y dimension at lower and upper ends, replicating
 *        data assuming wraparound.  This is primarily to handle polar
 *        grids with full 360 degrees in y.
 */

#ifndef GRID_EXPAND_H
#define GRID_EXPAND_H
#include <euclid/Grid2d.hh>

//------------------------------------------------------------------
class GridExpand : public Grid2d
{
public:
  /**
   * Empty constructor
   */
  GridExpand(void);

  /**
   * Constructor
   * @param[in] g  A Grid, not expanded
   * @param[in] ny  The number of y values to expand by at each end.
   */
  GridExpand(const Grid2d &g, const int ny);

  /**
   * Destructor
   */
  virtual ~GridExpand(void);

  /**
   * Put gdata into the expanded local grid
   *
   * @param[in] gdata  Grid to expand, not expanded
   * @param[in] doesWraparound  True if input grid wraps around in y dimension
   *
   * If doesWraparound, the expanded rows are give appropriate wrapped around
   * values, otherwise the expanded rows are all missing data.
   */
  void fill(const Grid2d &gdata, const bool doesWraparound);

  /**
   * @return true if x,y indices (from the nonexpanded grid space) are interior
   * to the expanded grid space.
   *
   * @param[in] x  X index value in non-expanded grid
   * @param[in] y  Y index value in non-expanded grid
   */
  bool xyIsInExpandRange(const int x, const int y) const;

  /**
   * Get value from local expanded grid using unexpanded input indices
   * @param[in] x  X index value in non-expanded grid
   * @param[in] y  Y index value in non-expanded grid
   * @param[out] v  Value returned
   * @return true if success
   */
  bool xyGetValue(const int x, const int y, double &v) const;

  /**
   * @return true if y (nonexpanded grid) is interior to local (expanded) grid
   * @param[in] y  Y index value in non-expanded grid
   */
  bool yIsInExpandRange(const int y) const;

  /**
   * @return true if x (nonexpanded grid) is interior to local (expanded) grid
   * @param[in] x  X index value in non-expanded grid
   */
  bool xIsInExpandRange(const int x) const;
  
protected:
private:

  /**
   * Expansion number of grid points on each y end of the grid
   */
  int _expandNy;
};

#endif
