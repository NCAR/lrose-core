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
 * @file Grid2dEdgeBuilder.hh
 * @brief An algorithm that follows an edge in a grid.
 * @class Grid2dEdgeBuilder
 * @brief An algorithm that follows an edge in a grid.
 */

# ifndef    GRID2D_EDGEBUILDER_H
# define    GRID2D_EDGEBUILDER_H

#include <euclid/Grid2d.hh>
#include <vector>
#include <string>

//------------------------------------------------------------------
class Grid2dEdgeBuilder 
{
 public:

  /**
   * Constructor
   *
   * @param[in] g Grid to use as template
   */
  Grid2dEdgeBuilder(const Grid2d &g);

  /**
   * Destructor
   */
  virtual ~Grid2dEdgeBuilder(void);

  /**
   * Add input grid index to the local state as the next edge point.
   * Typically this is called for successive endpoints of a polygonal closed
   * shape.
   *
   * @param[in] x  Grid index
   * @param[in] y  Grid index
   *
   * @note fills gaps as it builds the edge, (intermediate edge 
   *       points are added using line segment).
   *       This therefore assumes calls to
   *       this method are sequential while traversing a polygon.
   */
  void addVertex(const int x, const int y);

  /**
   * @return true if no points have been added using addVertex()
   */
  bool bad(void) const;
  
  /**
   * @return range of x,y indices in the edge
   *
   * @param[out] x0
   * @param[out] x1
   * @param[out] y0
   * @param[out] y1
   */
  inline void getRange(int &x0, int &x1, int &y0, int &y1) const
  {
    x0 = _x0;
    x1 = _x1;
    y0 = _y0;
    y1 = _y1;
  }

  /**
   * return pointer to the edge data. This grid is missing everywhere
   * except at the edge points.
   */
  inline const Grid2d *getEdgePtr(void) const { return &_edge;}

protected:
private:

  Grid2d _edge; /**< Grid containing the edge data (GOOD or BAD),
		  *    GOOD on the edge points, BAD everywhere else */
  int _x0;      /**< minimum x index */
  int _x1;      /**< maximum x index */
  int _y0;      /**< minimum y index */
  int _y1;      /**< maximum y index */
  int _last_x;  /**< last point added */
  int _last_y;  /**< last point added */

  void _fillGaps(const int x, const int y);
};

# endif
