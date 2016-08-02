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
 * @file RadxBeamBlock.hh
 * @brief  The algorithm
 * @class RadxBeamBlock
 * @brief  The algorithm
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 *
 * RadxBeamBlock reads an input Digital Elevation file, and
 * produces beam blockage information for a radar volume as configured 
 * and writes out results in formats supported by Radx.
 */

#ifndef RADXBEAMBLOCK_HH
#define RADXBEAMBLOCK_HH

#include "Args.hh"
#include "Parms.hh"
#include "VolHandler.hh"
#include "DigitalElevationHandler.hh"
#include <BeamBlock/beam_power.h>
#include <BeamBlock/beam_propagation.h>

class GridAlgs;
class Grid2d;
class ScanHandler;
class RayHandler;


//---------------------------------------------------------------------------
class RadxBeamBlock
{
  
public:
  /**
   * @param[in] parms   Alg parameters
   */
  RadxBeamBlock (const Parms &parms);

  /**
   * Destructor
   */
  ~RadxBeamBlock(void);

  /**
   * Run the algorithm, 
   * @return 1 for failure, 0 for succes
   */
  int Run(void);

  /**
   * Wite out results
   * @return 1 for failure, 0 for succes
   */
  int Write(void);

protected:
private:

  Parms _params;                /**< Alg parameters */
  VolHandler _data;             /**< Handles creation and writing of Radx*/
  DigitalElevationHandler _dem; /**< handles reading and use of digital
				 * elevation data */

  bool _processScan(ScanHandler &scan,
		    const rainfields::ancilla::beam_power &power_model,
		    rainfields::latlonalt origin, bool &short_circuit);
  void _processBeam(RayHandler &ray, rainfields::latlonalt origin, 
		    const rainfields::ancilla::beam_propagation &bProp,
		    const rainfields::ancilla::beam_power_cross_section &csec);
  void _processGate(GateHandler &ray, rainfields::angle elevAngle, size_t iray,
		    rainfields::latlonalt origin,
		    const rainfields::ancilla::beam_propagation &bProp,
		    rainfields::angle bearing,
		    const rainfields::ancilla::beam_power_cross_section &csec,
		    rainfields::angle &max_ray_theta);
  void _adjustValues(size_t ray,
		     const rainfields::ancilla::beam_propagation &bProp,
		     rainfields::real peak_ground_range, 
		     rainfields::real peak_altitude,
		     rainfields::angle elevAngle,
		     const rainfields::ancilla::beam_power_cross_section &csec,
		     rainfields::angle &max_ray_theta, 
		     rainfields::real &progressive_loss);

};

#endif

