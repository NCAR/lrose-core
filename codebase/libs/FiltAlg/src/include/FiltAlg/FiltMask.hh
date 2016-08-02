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
 * @file FiltMask.hh 
 * @brief Set data within a grid to missing using another grid as mask
 * @class FiltMask
 * @brief Set data within a grid to missing using another grid as mask
 *
 * The main input is the data to filter within the mask. The second input
 * is the mask data. The inputs must be Data::GRID3D.  The one output is
 * the filtered data.
 */

#ifndef FILT_MASK_H
#define FILT_MASK_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>

//------------------------------------------------------------------
class FiltMask : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltMask(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltMask(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  /**
   * Name of mask input field
   */
  string _mask_name;

  /**
   * True if mask data is input to the program, false if derived upstream
   */
  bool _mask_is_input;

  /**
   * Pointer to the mask input data.
   */
  const Data *_mask;

  /**
   * Ranges of mask values that indicating masking should happen,
   * first = lower limit, second = upper limit.
   * Masking can happen in more than one range of values.
   */
  vector<pair<double,double> > _range;

  /**
   * Add data to ranges
   *
   * @param[in] n  Number of entries in r array
   * @param[in] r  Array with range information
   */
  void _set_ranges(const int n, const FiltAlgParams::mask_range_t *r);
};

#endif
