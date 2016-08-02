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
 * @file DigitalElevationHandler.hh
 * @brief handles reading and use of digital elevation file
 * @class DigitalElevationHandler
 * @brief handles reading and use of digital elevation file
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 */

#ifndef DIGITALELEVATIONHANDLER_HH
#define DIGITALELEVATIONHANDLER_HH

#include "Parms.hh"
#include <BeamBlock/digital_elevation.h>
#include <BeamBlock/spheroid.h>
#include <vector>


class DigitalElevationHandler
{
public:

  /**
   * Constructor
   * @param[in] params
   */
  DigitalElevationHandler (const Parms &params);

  /**
   * Destructor
   */
  ~DigitalElevationHandler(void);

  /**
   * Initialize for the input range of lat/lons
   * @param[in] sw  The 'southwest' latlon  (first=lat, second=lon)
   * @param[in] ne  The 'northeast' latlon   (first=lat, second=lon)
   *
   * @return true for success
   */
  bool set(const std::pair<double,double> &sw,
	   const std::pair<double,double> &ne);

  /**
   * convert the site location of the volume into the native spheroid of the DEM
   * @param[in] radar  Site location
   *
   * @return native spheroid radar origin
   */
  rainfields::latlonalt radarOrigin(const rainfields::latlonalt &radar) const;

  /**
   * get peak ground range and peak altitude from inputs and local object
   * @param[in] origin  Radar loc
   * @param[in] bearing  The angle
   * @param[in] min_range  Minimum range for this gate (meters)
   * @param[in] max_range  Maximum range for this gate (meters)
   * @param[out] peak_ground_range  Range (meters) to peak max 
   * @param[out] peak_altitude  Altitude (meters) of peak
   * @param[in] bin_samples  Number of subsamples per bin
   */
  void determine_dem_segment_peak(const rainfields::latlon& origin,
				  rainfields::angle bearing,
				  rainfields::real min_range,
				  rainfields::real max_range,
				  rainfields::real& peak_ground_range,
				  rainfields::real& peak_altitude,
				  size_t bin_samples
				  ) const;

  double getElevation(const rainfields::latlon& loc) const;

  bool isOK; /**< True for object ok */

protected:
  
private:

  /**
   * Alg parameters
   */
  Parms _params;

  /**
   * the DEM handler
   */
  std::unique_ptr<rainfields::ancilla::digital_elevation> _dem;

  void _set(const std::pair<double,double> &sw,
	    const std::pair<double,double> &ne, 
	    rainfields::ancilla::spheroid::standard which);

  
};

#endif

