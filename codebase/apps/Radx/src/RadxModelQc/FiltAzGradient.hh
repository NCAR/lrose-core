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
 * @file FiltAzGradient.hh 
 * @brief Apply the azimuth gradient filter
 * @class FiltAzGradient
 * @brief Apply the azimuth gradient filter
 *
 *    data(k+1) - data(k-1)/(az(k+1)-az(k-1))/2
 */

#ifndef FILT_AZ_GRADIENT_H
#define FILT_AZ_GRADIENT_H

#include "Params.hh"
#include "Filter.hh"
#include "AzGradientState.hh"

class RadxSweep;

//------------------------------------------------------------------
class FiltAzGradient : public Filter
{
public:

  /**
   * Constructor
   * @param[in] f  Parameters for particular filter
   * @param[in] p  General params
   */
  FiltAzGradient(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~FiltAzGradient(void);

  #include "FilterVirtualFunctions.hh"

protected:
private:

  Params::math_e _type;   /**< type of math operation to do to data */
  double _value;          /**< value to use in math operation */

  bool _stateSet;         /**< True if _state is valid */
  std::vector<AzGradientState> _state;  /**< state, one item per sweep */

  void _setState(const RadxSweep &s, const RadxVol &vol);
  double _deltaAngleDegrees(const RadxSweep &s, const RadxVol &vol,
			    double &angle0, double &angle1) const;
  bool _set2Rays(const RadxRay *ray0, const RadxRay *ray1, 
		 const RadxRay &ray, const RadxRay **_ray0,
		 const RadxRay **_ray1) const;
  bool _setPreviousRayWhenNewSweep(int sweep, int nextSweep,
				   const RadxRay **lray0,
				   const RadxRay **lray1) const;
  bool _setNextRayWhenEndSweep(int sweep, int nextSweep, const RadxRay **lray0,
			       const RadxRay **lray1) const;
  bool _veryFirstRay(const RadxRay &ray, const RadxRay *ray1,
		     const RadxRay **lray0, const RadxRay **lray1) const;
  bool _veryLastRay(const RadxRay *ray0, const RadxRay &ray,
		    const RadxRay **lray0, const RadxRay **lray1) const;
};

#endif
