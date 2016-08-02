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
 * @file FiltSpectrumWidthNorm.hh 
 * @brief Apply the spectrum width normalization equations
 * @class FiltSpectrumWidthNorm
 * @brief Apply the spectrum width normalization equations
 *
 */

#ifndef FILT_SPECTRUM_WIDTH_NORM_HH
#define FILT_SPECTRUM_WIDTH_NORM_HH

#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class FiltSpectrumWidthNorm : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltSpectrumWidthNorm(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltSpectrumWidthNorm(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  bool _dbz;               /**< DBZ or VR */
  double _lambda;          /**< Param */

  double _meanPrt;         /**< Mean PRT set from input volume */
  double _meanNsamples;    /**< Mean number of samples set from input vol */
  bool _meanValuesSet;     /**< true if meanPrt and meanNsamples are set */
};

#endif
