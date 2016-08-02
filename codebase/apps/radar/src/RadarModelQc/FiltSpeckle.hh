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
 * @file FiltSpeckle.hh 
 * @brief Mask out speckle
 * @class FiltSpeckle
 * @brief Mask out speckle
 *
 * Set output to one of 2 values, or missing.
 */

#ifndef FILT_SPECKLE_H
#define FILT_SPECKLE_H

#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class FiltSpeckle : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltSpeckle(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltSpeckle(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  bool _outlierExceedsSurroundings; /**< True if outlier is only larger
				     * than surroundings, false if can be
				     * smaller */
  double _outlierThresh;  /**< minimum change in data along a beam to be an
			   * outlier */
  double _speckleConfidence; /**< Value to use if speckle detected */
  double _nonspeckleConfidence; /**< Value to use if not speckle */

};

#endif
