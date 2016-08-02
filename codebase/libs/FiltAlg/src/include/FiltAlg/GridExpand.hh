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
 * @brief Expand a 2d grid at each end of the azimuth dimension
 * @class GridExpand
 * @brief Expand a 2d grid at each end of the azimuth dimension
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
   * @param[in] g  Grid non expanded
   * @param[in] ny  Number of y values to expand by at each end.
   */
  GridExpand(const Grid2d &g, const int ny);

  /**
   * Destructor
   */
  virtual ~GridExpand(void);

  /**
   * Put gdata into the bigger grid
   * @param[in] gdata  Grid to expand
   * @param[in] is_360  True if input grid is a full 360
   */
  void fill(const Grid2d &gdata, const bool is_360);

  /**
   * @return true if x,y (nonexpanded grid) is interior to local (expanded) grid
   * @param[in] x  X value in non-expanded grid
   * @param[in] y  Y value in non-expanded grid
   */
  bool in_range_nonexpand(const int x, const int y) const;

  /**
   * Get value from expanded grid using unexpanded indices
   * @param[in] x  Unexpanded grid x index
   * @param[in] y  Unexpanded grid y index
   * @param[out] v  Value returned
   * @return true if success
   */
  bool get_value_nonexpand(const int x, const int y, double &v) const;

  /**
   * @return true if y (nonexpanded grid) is interior to local (expanded) grid
   * @param[in] y  Unexpanded y value
   */
  bool y_in_range_nonexpand(const int y) const;

  /**
   * @return true if x (nonexpanded grid) is interior to local (expanded) grid
   * @param[in] x  Unexpanded x value
   */
  bool x_in_range_nonexpand(const int x) const;
  
protected:
private:

  /**
   * Expansion number of grid points on each y end of the grid
   */
  int _expand_ny;
};

#endif
