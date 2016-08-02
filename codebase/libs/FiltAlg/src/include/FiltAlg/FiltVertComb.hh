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
 * @file FiltVertComb.hh 
 * @brief Filter to combine vertical levels down to 1 value.
 * @class FiltVertComb 
 * @brief Filter to combine vertical levels down to 1 value.
 *
 * Reduces 2d data to 1d, by taking average, max, or product.
 * The input is Data of type Data::DATA2D, The output is Data of type
 * Data::DATA1D.
 */

#ifndef FILT_VERT_COMB_H
#define FILT_VERT_COMB_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>

//------------------------------------------------------------------
class FiltVertComb : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltVertComb(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltVertComb(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  // double _v;     /**< Current partial result (depends on filtering action) */
  // double _count; /**< Current counter used for VERT_AVERAGE */
};

//------------------------------------------------------------------
/**
 * @class VertCombExtra
 * @brief Extra information needed for this filter, = 1 value
 *
 */
class VertCombExtra
{
public:
  /**
   * Constructor, sets value to 0
   */
  inline VertCombExtra(void) { _v = 0;}

  /**
   * Destructor
   */
  inline ~VertCombExtra(void) {}
  
  double _v;     /**< The single value for a vlevel */
};


#endif

