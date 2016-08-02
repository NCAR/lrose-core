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
 * @file Grid2dLoop.hh 
 * @brief Traverse a Grid2d in a way that the difference is only one index
 * at a time in x or y but not both.
 *
 * @class Grid2dLoop 
 * @brief Traverse a Grid2d in a way that the difference is only one index
 * at a time in x or y but not both.
 *
 * @note Move up then right, then down, then right, then up, etc.
 * The class was created for efficient averaging techniques that allow
 * subtraction and addition of only a subset of values for each step.
 * 
 */

#ifndef GRID2DLOOP_H
#define GRID2DLOOP_H

#include <string>
#include <vector>

//------------------------------------------------------------------
class Grid2dLoop
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
   * @param[in] nx The Grid2d size
   * @param[in] ny The Grid2d size
   */
  Grid2dLoop(const int nx, const int ny);

  /**
   * Destructor
   */
  virtual ~Grid2dLoop(void);

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
   * Increment by one gridpoint 
   *
   * @return true if still moving, false if all done
   */
  bool increment(void);

  /**
   * Return the set of x,y that are new compared to previous state,
   * for two dimensional filters.  
   *
   * @param[in] sx  Filter width x
   * @param[in] sy  Filter width y
   *
   * @return vector of x,y pairs that are new
   */
  std::vector<std::pair<int,int> > newXy(const int sx, const int sy) const;

  /**
   * return the set of x,y that have just been removed compared to the previous
   * state for two dimensional filters
   *
   * @param[in] sx  Filter width x
   * @param[in] sy  Filter width y
   *
   * @return vector of x,y pairs that have been removed.
   */
  std::vector<std::pair<int,int> > oldXy(const int sx, const int sy) const;

  /**
   * @return A string description of a state value
   *
   * @param[in] s  State 
   */
  static std::string printState(const State_t s);

protected:
private:

  int _nx;  /**< Grid size */
  int _ny;  /**< Grid size */

  int _x;         /**< Current location*/
  int _y;         /**< Current location*/
  State_t _state; /**< Current state = type of motion */

};

#endif
