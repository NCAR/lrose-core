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
#include <toolsa/copyright.h>
/**
 * @file Info.hh
 * @brief The Information (in and out) of the algorithm
 * @class Info
 * @brief The Information (in and out) of the algorithm
 */

# ifndef    INFO_HH
# define    INFO_HH

#include <ctime>
class RadxRay;
class RadxPersistentClutter;

//----------------------------------------------------------------
class Info
{
public:

  /**
   * Default constructor, values are not set
   */
  Info(void);
  
  /**
   * Constructor that sets all member values
   * @param[in] t  Time of data
   * @param[in] ray  Pointer to ray
   * @param[in] alg  Pointer to algorithm
   */
  Info(const time_t &t, const RadxRay *ray, RadxPersistentClutter *alg);

  /**
   * Set all member values
   * @param[in] t  Time of data
   * @param[in] ray  Pointer to ray
   * @param[in] alg  Pointer to algorithm
   */
  void set(const time_t &t, const RadxRay *ray, RadxPersistentClutter *alg);


  /**
   * Destructor
   */
  virtual ~Info(void);

  time_t _time;              /**< Data time, set to 0 if values are not set */
  const RadxRay *_ray;       /**< Pointer to a ray */
  RadxPersistentClutter *_alg; /**< Pointer to main algorithm */

protected:
private:

};

#endif
