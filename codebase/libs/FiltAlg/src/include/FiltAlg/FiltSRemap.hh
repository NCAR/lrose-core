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
 * @file FiltSRemap.hh 
 * @brief Remap Data using a fuzzy function
 * @class FiltSRemap
 * @brief Remap Data using a fuzzy function
 *
 * The one input is data to remap. The one output is data of the same type
 * with all values remapped using a fuzzy function. Any type of data
 * is allowed.
 */

#ifndef FILT_S_REMAP_H
#define FILT_S_REMAP_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>
#include <rapmath/SFuzzyF.hh>

//------------------------------------------------------------------
class FiltSRemap : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltSRemap(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltSRemap(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
 private:

  /**
   * The fuzzy function that maps input values to output values
   */
  SFuzzyF _f; 


  /**
   * perform the remapping for input data
   * @param[in,out] o  The input data filtered on exit
   * @return true if successfully mapped
   */
  bool _remap(FiltInfoOutput &o) const;

};

#endif
