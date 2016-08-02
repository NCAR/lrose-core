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
 * @file Grid2dInside.hh
 * @brief Algorithm to distinguish points that are inside or outside a region
 * @class Grid2dInside
 * @brief Algorithm to distinguish points that are inside or outside a region
 *        
 *
 * Using an EdgeBuilder object, which has all 'edge' points defined, a subset
 * grid that just contains the edge points is defined, then evaluated using a 
 * 'crossing' algorithm to find inside and outside points.
 */

# ifndef    GRID2D_INSIDE_H
# define    GRID2D_INSIDE_H

class Grid2dEdgeBuilder;
#include <euclid/Grid2d.hh>

//------------------------------------------------------------------
class Grid2dInside 
{
 public:

  /**
   * Constructor
   *
   * @param[in] e Edgebuilder, with its state filled in.
   *
   * @note the constructor does pretty much everything to identify
   * points that are on the inside.
   */
  Grid2dInside(const Grid2dEdgeBuilder &e);

  /**
   * Destructor
   */
  virtual ~Grid2dInside(void);

  /**
   *@return number of gridpoints (x) in the subset of the grid being evaluated
   */
  inline int nx(void) const {return _nx;}

  /**
   *@return number of gridpoints (y) in the subset of the grid being evaluated
   */
  inline int ny(void) const {return _ny;}

  /**
   * @return true if input point is an interior (inside) point
   *
   * @param[in] x  X index into the subset indices (x=i means acual grid
   *               index=_x0+i)
   * @param[in] y  Y index into the subset indices (y=i means actual grid
   *               index=_y0+i)
   * @param[out] xi  Corresponding X index in the full grid
   * @param[out] yi  Corresponding X index in the full grid
   */
  bool inside(const int x, const int y, int &xi, int &yi) const;

protected:
private:

  int _nx;  /**< Subset grid number of points x */
  int  _ny; /**< Subset grid number of points y */

  Grid2d _g; /**< Grid over the subset, with missing everywhere except at
	      * points that are 'inside' */

  int _x0; /**< lower left corner of subset grid in full grid */
  int _y0; /**< lower left corner of subset grid in full grid */
  int _x1; /**< upper right corner of subset grid in full grid */
  int _y1; /**< upper right corner of subset grid in full grid */

};

# endif
