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
class RadxTimeMedian;

//------------------------------------------------------------------
class Info
{
public:

  /**
   * constructor, sets members
   * @param[in] input_field
   * @param[in] ray
   * @param[in] alg
   */
  Info(const std::string &input_field, const RadxRay *ray,
       RadxTimeMedian *alg);

  /**
   * Destructor
   */
  virtual ~Info(void);


  std::string _input_field; /**< Name of input field */
  const RadxRay *_ray;      /**< Pointer to ray */
  RadxTimeMedian *_alg;     /**< Pointer to algorithm */

protected:
private:  
  
};

# endif
