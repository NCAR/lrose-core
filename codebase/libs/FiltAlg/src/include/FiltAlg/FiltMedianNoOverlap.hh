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
 * @file FiltMedianNoOverlap.hh 
 * @brief Median filter
 * @class FiltMedianNoOverlap
 * @brief Median filter
 *
 * A Median smoothing Filter on gridded data.  The filter Uses histogram
 * binning of data for efficiency. 
 * There is no overlap (i.e. one value replacated for each box, with a
 * resolution equal to the box size). One value is computed within the
 * box to generate the output at box center. 
 * The input is data to filter, the output is filtered data.
 */

#ifndef FILT_MEDIAN_NO_OVERLAP_H
#define FILT_MEDIAN_NO_OVERLAP_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>

//------------------------------------------------------------------
class FiltMedianNoOverlap : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltMedianNoOverlap(const FiltAlgParams::data_filter_t f,
		      const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltMedianNoOverlap(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  int _nr;     /**< 2d filter number of radial gates */
  int _ntheta; /**< 2d filter number of azimuthal gates */

  double _bin_min;  /**< Minimum bin value (histograms for efficiency) */
  double _bin_max;  /**< Maximum bin value (histograms for efficiency) */
  double _bin_delta;/**< Bin delta value (histograms for efficiency) */
};

#endif
