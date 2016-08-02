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
 * @file  ScanHandler.hh
 * @brief Handles storage of data that is created for one scan
 * @class ScanHandler
 * @brief Handles storage of data that is created for one scan
 */

#ifndef SCANHANDLER_HH
#define SCANHANDLER_HH

#include "Parms.hh"
#include "RayHandler.hh"
#include <BeamBlock/angle.h>
#include <string>
#include <vector>

class ScanHandler
{
public:

  /**
   * @param[in] elev  Elevation angle degrees
   * @param[in] params
   */
  ScanHandler (double elev, const Parms &params);

  /**
   * Destructor
   */
  ~ScanHandler(void);

  /**
   * @return elevation angle as an angle object
   */
  inline rainfields::angle elevation(void) const
  {
    rainfields::angle a;  a.set_degrees(_elev); return a;
  }

  /**
   * @return integer index into parameterized scans 0=lowest
   */
  inline int elevIndex(void) const {return _params.elevToIndex(_elev);}

  /**
   * @return elevation angle in degrees
   */
  inline double elevDegrees(void) const {return _elev;}

  /**
   * Apply Finishing steps to the data
   */
  void finish(void);

  typedef vector<RayHandler>::iterator iterator;
  typedef vector<RayHandler>::const_iterator const_iterator;
  typedef vector<RayHandler>::reverse_iterator reverse_iterator;
  typedef vector<RayHandler>::const_reverse_iterator const_reverse_iterator;
  auto begin() -> iterator                       { return _ray.begin(); }
  auto begin() const -> const_iterator           { return _ray.begin(); }
  auto end() -> iterator                         { return _ray.end(); }
  auto end() const -> const_iterator             { return _ray.end(); }


  bool isOK;  /**< True if object  is good */

protected:
  
private:

  double _elev;                  /**< angle (degrees) */
  Parms _params;                 /**< Parameters */
  std::vector<RayHandler> _ray;  /**< The data created, one per ray */
};

#endif

