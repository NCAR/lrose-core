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
 * @file  GateHandler.hh
 * @brief Handles storage of data for one gate
 * @class GateHandler
 * @brief Handles storage of data for one gate
 */
#ifndef GATEHANDLER_HH
#define GATEHANDLER_HH

#include "Params.hh"
#include <Radx/Radx.hh>

class GateHandler
{
public:

  /**
   * @param[in]  gateMeters  The distance the gate is from the radar (meters)
   */
  GateHandler (double gateMeters);

  /**
   * Destructor
   */  
  ~GateHandler(void);

  /**
   * @return distance in meters gate is from radar
   */
  inline double meters(void) const {return _gateMeters;}

  /**
   * If input peak is higher than current stored value, replace stored value
   * with input
   * @param[in] peak  Elevation meters
   */
  inline void adjustPeak(double peak) {if (_peak < peak) _peak = peak;}

  /**
   * Add input loss to the beam blockage local value
   * @param[in] loss
   */
  inline void incrementLoss(double loss) {_beamb += loss;}

  /**
   * Apply finishing calculations to the data
   */
  void finish(void);

  /**
   * @return the locally stored data value for the input type
   * @param[in] type  The type
   */
  Radx::fl32 getData(Params::output_data_t type) const;

  
  bool isOK;  /**< True if object  is good */

protected:
  
private:

  double _gateMeters;   /**< Distance of gate from radar (meters) */
  double _beamb;        /**< Beam blockage db */
  double _beaml;        /**< Beam blockage linear */
  double _peak;         /**< Peak elevation */
};

#endif
