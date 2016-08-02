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
 * @file FiltCombine.hh 
 * @brief filter to combine Data
 * @class FiltCombine
 * @brief filter to combine Data
 *
 * Combines Data from more than one input source. The methods of combining
 * are several, including MAX, AVERAGE, PRODUCT, WEIGHTED_SUM,
 * NORM_WEIGHTED_SUM.  The Comb member _comb does the bulk of the work.
 * This filter can handle any kind of input data, as long it is consistent
 * amongst inputs, Data::GRID3d combines at each grid point, Data::DATA2D
 * combines the single value at each vertical level, and Data::DATA1D combines
 * single values from each input.  
 *
 * The main input is any one of the data's to be combined, the additional inputs
 * are the other data's to be combined.  The single output is the result of
 * combining.
 */

#ifndef FILT_COMBINE_H
#define FILT_COMBINE_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>
#include <FiltAlg/Comb.hh>

//------------------------------------------------------------------
class FiltCombine : public Filter
{
public:
  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   *
   * @note uses params to initialize the Comb object _comb
   */
  FiltCombine(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltCombine(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  double _weight0;  /**< Weight to give the main input */
  Comb _comb;       /**< All the other inputs */

  /**
   * Do the filter when it is a vlevel slice
   * @param[in] inp  The input data
   * @param[out] o  Results are put in here.
   * @return true if successful
   */
  bool _filter_slice(const FiltInfoInput &inp, FiltInfoOutput &o) const;

  /** 
   * Do the filter when it is 1d single valued data
   * @param[in] in  The Data
   * @param[in] out  Output data
   * @param[out] o  Results are put here
   * @return true if successful
   */
  bool _filter_data1d(const Data &in, const Data &out, FiltInfoOutput &o) const;

  /**
   * Compare input data with output type and make sure everything is consistent.
   * @param[in] type  The output data type
   * @param[in] data  The input data
   * @return true if there is a match
   */
  bool _check_data(const Data::Data_t type, const VlevelSlice &data) const;
};

#endif
