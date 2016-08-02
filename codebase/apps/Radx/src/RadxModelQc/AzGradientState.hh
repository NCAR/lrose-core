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
 * @file AzGradientState.hh 
 * @brief Information needed for wraparound situation
 * @class AzGradientState
 * @brief Information needed for wraparound situation
 *
 *  Simple struct-like class
 */

#ifndef AZ_GRADIENT_STATE_H
#define AZ_GRADIENT_STATE_H

#include <cstdio>

//------------------------------------------------------------------
class AzGradientState
{
public:

  /**
   * Constructor for case of a full 360
   *
   * @param[in] f  Fixed angle
   * @param[in] s  Sweep Number
   * @param[in] r0  Index to 0th ray in sweep
   * @param[in] r1  Index to last ray in sweep
   * @param[in] ray0  Pointer to 0'th ray in sweep
   * @param[in] ray1  Pointer to last ray in sweep
   */
  inline AzGradientState(double f, int s, int r0, int r1, const RadxRay *ray0,
			 const RadxRay *ray1) :
    _fixedAngle(f), _sweepNumber(s), _is360(true), _r0(r0), _r1(r1), _ray0(ray0),
    _ray1(ray1) {}

  /**
   * Constructor for case of not a full 360
   *
   * @param[in] f  Fixed angle
   * @param[in] s  Sweep Number
   */
  inline AzGradientState(double f, int s) : 
    _fixedAngle(f), _sweepNumber(s), _is360(false), _r0(0), _r1(0), _ray0(NULL),
    _ray1(NULL) {}

  /**
   * Destructor
   */
  inline virtual ~AzGradientState(void) {}

  /**
   * Debug print
   */
  inline void print(void) const
  {
    if (_is360)
    {
      printf("elev:%lf, sweep:%d  r0:%d r1:%d IS360\n", _fixedAngle,
	     _sweepNumber, _r0, _r1);
    }
    else
    {
      printf("elev:%lf, sweep:%d  SECTOR\n", _fixedAngle, _sweepNumber);
    }
  }

  /**
   * @return true for a full 360
   */
  inline bool is360(void) const {return _is360;}

  double _fixedAngle;    /**< elevation angle degrees */
  int _sweepNumber;      /**< Sweep number */
  bool _is360;           /**< True for a full 360 */
  int _r0;               /**< Ray index 0 when full 360 */
  int _r1;               /**< Ray index last when full 360 */
  const RadxRay *_ray0;  /**< Pointer to ray 0 when full 360 */
  const RadxRay *_ray1;  /**< Pointer to last ray when full 360 */

protected:
private:

};

#endif
