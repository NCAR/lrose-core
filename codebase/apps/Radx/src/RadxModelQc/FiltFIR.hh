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
 * @file FiltFIR.hh 
 * @brief Masks data into two values based on input
 * @class FiltFIR
 * @brief Masks data into two values based on input
 */

#ifndef FILT_FIR_H
#define FILT_FIR_H
#include "Params.hh"
#include "Filter.hh"
#include <Radx/RayxData.hh>

//------------------------------------------------------------------
class FiltFIR : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltFIR(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltFIR(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  std::vector<double> _coeff;
  RayxData::FirFilter_t _edge;
  std::string _outputNoiseField;

  bool _coeffSet(int index, const Params &P);
  bool _coeffSet(int n, double *v);
};

#endif
