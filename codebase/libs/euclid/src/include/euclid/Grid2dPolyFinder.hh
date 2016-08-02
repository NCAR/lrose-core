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
 * @file Grid2dPolyFinder.hh
 * @brief Algorithm that follows an edge in a grid.
 * @class Grid2dPolyFinder
 * @brief Algorithm that follows an edge in a grid.
 *
 * First the edge is put to a grid, then the grid is traversed to build
 * the edge, starting at the lowest y value minimum x, pointing right.
 *
 * Tersely described, the algorithm is:
 *
 * When pointing right (ZERO_ONE):
 *   . Is right and down an edge? 
 *      .  move there, new state is pointing down (THREE_ZERO).
 *   . Else Is right an edge?
 *      .  move there, new state is pointing right (ZERO_ONE).
 *   . else 
 *      . change state to pointing up (ONE_TWO), no movement
 *
 * When pointing up (ONE_TWO):
 *   . Is right and up an edge? 
 *      .  move there, new state is pointing right (ZERO_ONE)
 *   . Else Is up an edge?
 *      .  move there, new state is pointing up (ONE_TWO)
 *   . else 
 *      . change state to pointing left (TWO_THREE), no movement
 *
 * When pointing left (TWO_THREE):
 *   . Is left and up an edge? 
 *      .  move there, new state is pointing up (ONE_TWO)
 *   . Else Is left an edge?
 *      .  move there, new state is pointing left (TWO_THREE)
 *   . else 
 *      . change state to pointing down (THREE_ZERO), no movement
 *
 * When pointing down (THREE_ZERO):
 *   . Is left and down an edge? 
 *      .  move there, new state is pointing left (TWO_THREE).
 *   . Else Is down an edge?
 *      .  move there, new state is pointing down (THREE_ZERO)
 *   . else 
 *      . change state to pointing right (ZERO_ONE), no movement
 *
 * This will traverse the edge, building the edge points.
 *
 * There can be 'spokes' which are removed by removeLines()
 *
 */

# ifndef    GRID2D_POLYFINDER_H
# define    GRID2D_POLYFINDER_H

#include <euclid/GridAlgs.hh>
#include <vector>
#include <string>

//----------------------------------------------------------------
class Grid2dPolyFinder 
{
 public:

  /**
   * Constructor
   */
  Grid2dPolyFinder(void);

  /**
   * Destructor
   */
  virtual ~Grid2dPolyFinder(void);

  /**
   * Debug brief print
   */
  void print(void) const;

  /**
   * Debug full print
   */
  void printFull(void) const;

  /**
   * Initialize 
   *
   * @param[in] g  Grid with one clump of non-missing data
   * @param[in] rescale  Amount to reduce resolution 
   *                       -  1  = none
   *                       -  2 = half
   *                       -  3 = one third, etc.
   * @return true for success
   *
   * Create _hr_edge = high res edge yes/no grid
   * Create _lr_edge = low res edge yes/no grid
   */
  bool init(const GridAlgs &g, const int rescale);

  /**
   * Move through the grid, building edge points into
   * local state using a search algorithm. 
   *
   * @eturn true if the next point is now defined, and update the
   * internal state, moving to this next point
   */
  bool next(void);

  /**
   * remove 'out and back lines' in the polygon
   * An out and back line is one that is a spoke of width 0,
   * which has no interior
   */
  void removeLines(void);

  /**
   * reduce the polygon to its convex hull
   */
  void createConvexHull(void);

  /**
   * @return size of the polygon
   */
  inline int num(void) const {return static_cast<int>(_x.size());}

  /**
   * @return i'th x,y index values for the polygon
   * @param[in] i  Index
   * @param[out] ix  X index
   * @param[out] iy  Y index
   */
  inline void getIth(const int i, int &ix, int &iy) const
  {
    ix = _x[i]*_rescale;
    iy = _y[i]*_rescale;
  }

  /**
   * @return the edge grid, which is missing at all non-edge points.
   */
  inline GridAlgs getEdge(void) {return _hr_edge;}

protected:
private:

  /**
   * @num state_e Enumeration of the possible states
   */
  typedef enum 
  {
    ZERO_ONE = 0,  /**< Pointing right */
    ONE_TWO = 1,   /**< Pointing up */
    TWO_THREE = 3, /**< Pointing left */
    THREE_ZERO = 4 /**< Pointing down */
  } state_e;

  std::vector<int> _x;  /**< The edge point indices, in order */
  std::vector<int> _y;  /**< The edge point indices, in order */
  int _x0;              /**< Current index */
  int _y0;              /**< Current index */
  state_e _state;       /**< Direction of motion */

  // lower resolution grid for computations
  GridAlgs _g;       /**< Low res data grid */
  GridAlgs _lr_edge; /**< Low res 'edge' grid with 'BAD' or 'GOOD' */
  GridAlgs _hr_edge; /**< High res 'edge' grid with 'BAD' or 'GOOD' */
  int _rescale;     /**< Amount to reduce resolution 
		     * -  1  = none
		     * -  2 = half
		     * -  3 = one third, etc.*/

  std::string _printState(const state_e e) const;
  void _removeLines(const int k);
  bool _isLine(const int i, const int k) const;
  void _removeLine(const int i, const int k);
  bool _incForLines(int &i, const int k) const;
};

# endif
