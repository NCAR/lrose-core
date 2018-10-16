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
 * @file Grid2dLoopA.hh 
 * @brief Traverse a Grid2d with a moving box in a way that the difference
 * is only one index at a time in x or y but not both, with a Grid2dLoopAlg
 * object handling the algorithm within the box.
 *
 * @class Grid2dLoopA 
 * @brief Traverse a Grid2d with a moving box in a way that the difference
 * is only one index at a time in x or y but not both, with a Grid2dLoopAlg
 * object handling the algorithm within the box.
 *
 * @note Move up then right, then down, then right, then up, etc.
 * The class was created for efficient averaging techniques that allow
 * subtraction and addition of only a subset of values for each step.
 */

#ifndef GRID2DLOOPA_H
#define GRID2DLOOPA_H

#include <string>
#include <vector>

class Grid2d;
class Grid2dLoopAlg;

//------------------------------------------------------------------
class Grid2dLoopA
{
public:

  /**
   * @enum State_t
   * @brief Can move up in Y, down in Y or right in X
   */
  typedef enum
  {
    INIT=0,     /**< Initial state */
    INC_Y=1,    /**< Moving up in Y */
    DEC_Y=2,    /**< Moving down in Y */
    INC_X=3     /**< Moving up in X */
  } State_t;

  /**
   * @param[in] nx  Number of Grid2d points x
   * @param[in] ny  Number of Grid2d points y
   * @param[in] sx  The box width (x)
   * @param[in] sy  The box width (y)
   */
  Grid2dLoopA(int nx, int ny, int sx, int sy);

  /**
   * Destructor
   */
  virtual ~Grid2dLoopA(void);

  /**
   * Set the state to initial state. This puts you in the lower left corner.
   */
  void reinit(void);

  /**
   * @return current state and current location
   *
   * @param[out] x  Current location x
   * @param[out] y  Current location y
   */
  State_t getXy(int &x, int &y) const;
  
  /**
   * Get the current location, and the algorithm result within the box
   * @param[in] alg  The algorithm state
   * @param[in] minGood  Minimum number of nonmissing points within the box
   * @param[out]  x  Current center point index of box
   * @param[out]  y  Current center point index of box
   * @param[out] result  The algorithm result in the box
   *
   * @return true for success, false for no more points or error
   */
  bool getXyAndResult(const Grid2dLoopAlg &alg, int minGood, int &x, int &y, 
		      double &result) const;

  /**
   * Increment by one gridpoint 
   *
   * @return true if still moving, false if all done
   *
   * @param[in] G  Grid2d with data
   * @param[in,out] data The algorithm state, updated
   */
  bool increment(const Grid2d &G, Grid2dLoopAlg &data);

  /**
   * @return string description of a state value
   *
   * @param[in] s  State 
   */
  static std::string printState(const State_t s);

protected:
private:



  int _nx;  /**< Grid size */
  int _ny;  /**< Grid size */

  int _x;         /**< Current centerpoint index */
  int _y;         /**< Current centerpoint index */

  int _minx;  /**< Current minimum x for the box */
  int _maxx;  /**< Current maximum x for the box */
  int _miny;  /**< Current minimum y for the box */
  int _maxy;  /**< Current maximum y for the box */

  int _sx; /**< Box width x */
  int _sy; /**< Box width y */


  State_t _state; /**< Current state = type of motion */

  void _fullCompute(const Grid2d &G, Grid2dLoopAlg &alg) const;
  void _addX(int y, const Grid2d &G, Grid2dLoopAlg &alg) const;
  void _subtractX(int y, const Grid2d &G, Grid2dLoopAlg &alg) const;
  void _addY(int y, const Grid2d &G, Grid2dLoopAlg &alg) const;
  void _subtractY(int y, const Grid2d &G, Grid2dLoopAlg &alg) const;
};

#endif
