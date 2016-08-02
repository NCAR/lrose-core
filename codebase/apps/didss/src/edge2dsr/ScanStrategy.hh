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
////////////////////////////////////////////////////////////////////////////
// $Id: ScanStrategy.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
//
///////////////////////////////////////////////////////////////////////////

/**************************************************************************
 * ScanStrategy - Class representing a scan strategy for a radar.
 *************************************************************************/


#ifndef ScanStrategy_hh
#define ScanStrategy_hh


#include <vector>
using namespace std;


class ScanStrategy
{

public:

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  ScanStrategy();
  virtual ~ScanStrategy();


  ////////////////////
  // Access Methods //
  ////////////////////

  /*************************************************************************
   * addElevation() - Add an elevation to the scan strategy.  Elevations must
   *                  be added in the order encountered in the radar volume.
   */

  inline void addElevation(const double elevation)
  {
    _elevations.push_back(elevation);
  }
  

  /*************************************************************************
   * getElevation() - Get the target elevation in the strategy at the
   *                  given tilt number or actual elevation.
   *
   * Returns the tilt elevation on success, -1.0 on failure.
   */

  inline double getElevation(const int tilt_num) const
  {
    return _elevations[tilt_num];
  }
  

  inline double getElevation(const double actual_elevation) const
  {
    int tilt_num = getTiltNum(actual_elevation);
    
    if (tilt_num < 0)
      return -1.0;
    
    return _elevations[tilt_num];
  }
  

  /*************************************************************************
   * getTiltNum() - Returns the tilt number for the given elevation.  If the
   *                given elevation is outside of all of the tilts, returns
   *                -1.
   */

  int getTiltNum(const double elevation) const;


private:

  vector< double > _elevations;
  
};

#endif
