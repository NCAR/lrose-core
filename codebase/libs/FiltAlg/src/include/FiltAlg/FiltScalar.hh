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
 * @file FiltScalar.hh 
 * @brief Data::GRID3D filtering, down to Data::DATA2D (one value per vlevel)
 * @class FiltScalar
 * @brief Data::GRID3D filtering, down to Data::DATA2D (one value per vlevel)
 *
 * This filter does some kind of reduction in values at each vlevel into
 * one value, such as averaging.  The one input is Data of type Data::GRID3D,
 * the one output is Data reduced to Data::DATA2D.
 */

#ifndef FILT_SCALAR_H
#define FILT_SCALAR_H
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/Filter.hh>

//------------------------------------------------------------------
class FiltScalar : public Filter
{
public:
  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltScalar(const FiltAlgParams::data_filter_t f, const FiltAlgParms &p);

  /**
   * Destructor
   */
  virtual ~FiltScalar(void);

  #include <FiltAlg/FilterVirtualFunctions.hh>

protected:
private:

  double _rLwr;      /**< Lower bound radius (physical dimension) */
  double _rUpr;      /**< Upper bound radius (physical dimension) */
  double _thetaLwr;  /**< Lower bound azimuth (physical dimension) */
  double _thetaUpr;  /**< Upper bound azimuth (physical dimension) */

};

#endif
