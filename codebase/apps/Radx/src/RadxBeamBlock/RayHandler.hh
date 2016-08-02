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
 * @file  RayHandler.hh
 * @brief Handles storage of data that is created for one ray
 * @class RayHandler
 * @brief Handles storage of data that is created for one ray
 */
#ifndef RAYHANDLER_HH
#define RAYHANDLER_HH

#include "GateHandler.hh"
#include <BeamBlock/angle.h>
#include <Radx/Radx.hh>
#include <string>
#include <vector>
class Parms;

class RayHandler
{
public:

  /**
   * @param[in] azimuth  Angle degrees
   * @param[in] elevation  Angle degrees
   * @param[in] parms
   */
  RayHandler (double azimuth, double elevation, const Parms &parms);

  /**
   * Destructor
   */
  ~RayHandler(void);

  /**
   * @return azimuth as an angle object
   */
  inline rainfields::angle azimuth(void) const
  {
    rainfields::angle a; a.set_degrees(_azimuth); return a;
  }

  /**
   * @return elevaiton as an angle object
   */
  inline rainfields::angle elev(void) const
  {
    rainfields::angle a; a.set_degrees(_elevation); return a;
  }

  /**
   * @return azimuth in degrees
   */
  inline double azDegrees(void) const {return _azimuth;}

  /**
   * @return elevation in degrees
   */
  inline double elevDegrees(void) const {return _elevation;}

  /**
   * Create and return an array of data for a particular type reprenting
   * that data type along the ray.
   * @param[in] type  The data type
   *
   * @return  The array
   *
   * @note this method allocates memory
   * @note the array length should equal the number of gates configured for
   */
  Radx::fl32 *createData(Params::output_data_t type) const;

  /**
   * Create and return an array of data for a particular type reprenting
   * that data type along the ray. At each point set value to max of all data
   * of that type at or closer than the value at the point along the ray.
   *
   * @param[in] type  The data type
   *
   * @return  The array
   *
   * @note this method allocates memory
   * @note the array length should equal the number of gates configured for
   */
  Radx::fl32 *createMaxData(Params::output_data_t type) const;


  /**
   * Apply Finishing steps to the data
   */
  void finish(void);


  typedef vector<GateHandler>::iterator iterator;
  typedef vector<GateHandler>::const_iterator const_iterator;
  typedef vector<GateHandler>::reverse_iterator reverse_iterator;
  typedef vector<GateHandler>::const_reverse_iterator const_reverse_iterator;
  auto begin() -> iterator                       { return _gate.begin(); }
  auto begin() const -> const_iterator           { return _gate.begin(); }
  auto end() -> iterator                         { return _gate.end(); }
  auto end() const -> const_iterator             { return _gate.end(); }

  bool isOK;  /**< True if object  is good */

protected:
  
private:

  double _azimuth;        /**< Ray azimuth degrees */
  double _elevation;      /**< Ray elevation angle degrees */
  std::vector<GateHandler> _gate;  /**< The data created, one per gate */
};

#endif

