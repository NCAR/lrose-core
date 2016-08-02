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
 * @file FiltRestrict.hh 
 * @brief Restricts data into two values based on second mask input
 * @class FiltRestrict
 * @brief Restricts data into two values based on second mask input
 */

#ifndef FILT_RESTRICT_H
#define FILT_RESTRICT_H
#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class FiltRestrict : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltRestrict(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltRestrict(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  string _mask_name;  /**< Name of second mask input */
  double _mask_value; /**< Value in mask data to keep main input as is */
  double _nonmask_replacement_value;  /**< value to set main input to when 
				       *  mask != mask_value,
				       * if _nonmask_is_missing = false
				       */
  bool   _nonmask_is_missing; /**< True to set main input to missing whereever
			       * mask != mask_value */
};

#endif
