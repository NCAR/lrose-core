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
 * @file Looper.hh 
 * @brief Increment in 2d so that change only one gridpoint in x or y in a
 *        systematic way
 *
 * @class Looper 
 * @brief Increment in 2d so that change only one gridpoint in x or y in a
 *        systematic way
 *
 * @note allows to move up then right, then down, then right, then up, ...
 * for efficiency.
 */

#ifndef LOOPER_H
#define LOOPER_H

//------------------------------------------------------------------
class Looper
{
public:

  /**
   * @enum Can move up in Y, down in Y or right in X
   */
  typedef enum
  {
    INIT=0,  /**< No motion yet */
    INC_Y=1, /**< Up in y */
    DEC_Y=2, /**< Down in y */
    INC_X=3  /**< Up in x */
  } State_t;

  /**
   * Constructor
   * @param[in] nx grid size
   * @param[in] ny grid size
   */
  Looper(const int nx, const int ny);

  /**
   * Destructor
   */
  virtual ~Looper(void);

  /**
   * Return current state and current location
   * @param[out] x  Current location X
   * @param[out] y  Current location Y
   * @return State values
   */
  State_t get_xy(int &x, int &y) const;
  
  /**
   * One step in the loop
   * @return true if still moving, false if all done
   */
  bool increment(void);


protected:
private:

  int _nx; /**< Grid size X */
  int _ny; /**< Grid size Y */
  int _x; /**< Current location X */
  int _y; /**< Current location Y */
  State_t _state;  /**< Current type of motion */
};

#endif
