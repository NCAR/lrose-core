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
 * @file Filt2dGaussianMapping.hh 
 * @brief function of 2 variables that gives a gaussian result
 * @class Filt2dGaussianMapping
 * @brief function of 2 variables that gives a gaussian result
 *
 * The equation is:
 * -   f(X,Y) = -1 - exp(-scale*(X*xfactor + y*yfactor))
 */

#ifndef FILT_2D_GAUSSIAN_MAPPING_HH
#define FILT_2D_GAUSSIAN_MAPPING_HH
#include "Params.hh"
#include "Filter.hh"

//------------------------------------------------------------------
class Filt2dGaussianMapping : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  Filt2dGaussianMapping(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~Filt2dGaussianMapping(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  std::string _y_field_name;  /**< name of input variable to use as "Y"
			       * (main input is used as "X") */
  double _x_factor;           /**< Value for equation */
  double _y_factor;           /**< value for equation */
  bool _x_is_absolute;        /**< True to take absolute value of X before
			       *   using in equation */
  bool _y_is_absolute;        /**< True to take absolute value of Y before
			       *   using in equation */
  double _scale;              /**< Value for equation */

};

#endif
