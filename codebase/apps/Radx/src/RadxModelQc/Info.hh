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
 * @file Info.hh
 * @brief Information passed in and out of algorithm
 * @class Info
 * @brief Information passed in and out of algorithm
 */

# ifndef   INFO_HH
# define   INFO_HH
#include <string>
class RadxRay;
class RadxModelQc;

//------------------------------------------------------------------
class Info
{
public:

  /**
   * constructor
   * @param[in] t  Data time
   * @param[in] ray  Pointer to ray information
   * @param[in] alg  Pointer to algorithm object
   */
  Info(const time_t &t, RadxRay *ray, RadxModelQc *alg);

  /**
   * constructor
   * @param[in] t  Data time
   * @param[in] ray0  Pointer to previous ray information
   * @param[in] ray  Pointer to ray information
   * @param[in] ray1  Pointer to next ray information
   * @param[in] alg  Pointer to algorithm object
   */
  Info(const time_t &t, RadxRay *ray0, RadxRay *ray, RadxRay *ray1,
       RadxModelQc *alg);

  /**
   *  destructor
   */
  virtual ~Info(void);


  time_t _time;  /**< Data time */
  RadxRay *_ray0; /**< Pointer to prior ray or empty */
  RadxRay *_ray; /**< Pointer to ray */
  RadxRay *_ray1; /**< Pointer to next ray or empty */
  RadxModelQc *_alg; /**< Pointer to algorithm */

protected:
private:  
  
};

# endif
