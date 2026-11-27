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
/////////////////////////////////////////////////////////////
// Beam - Class representing a single beam to be added to the sweep file.
//
// Nancy Rehak, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef Beam_H
#define Beam_H

#include <iostream>
#include <vector>

#include <RaycTime.h>
#include <RayFile.h>
#include <vectorDefs.h>

#include <rapformats/DsRadarMsg.hh>

using namespace std;


class Beam
{
  
public:

  /*********************************************************************
   * Constructors
   */
  
  Beam(const DsRadarMsg &radar_msg,
       bool debug_flag, bool verbose);


  /*********************************************************************
   * Destructor
   */
  
  virtual ~Beam();


  /*********************************************************************
   * addFieldData() - Add the given field data to the beam.
   */
  
  void addFieldData(RayDoubles *field_data)
  {
    _fieldData.push_back(field_data);
  }


  /*********************************************************************
   * addBeamToFile() - Add this beam to the given sweep file.
   */

  void addBeamToFile(RayFile *ray_file);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getTime() - Get the time value for this beam.
   */

  ForayUtility::RaycTime getTime() const
  {
    return _time;
  }
  

  /*********************************************************************
   * getAzimuth() - Get the azimuth value for this beam.
   */

  double getAzimuth() const
  {
    return _azimuth;
  }
  

  /*********************************************************************
   * getElevation() - Get the elevation value for this beam.
   */

  double getElevation() const
  {
    return _elevation;
  }
  

  /*********************************************************************
   * getTargetElev() - Get the target elevation value for this beam.
   */
  
  double getTargetElevation() const
  {
    return _targetElevation;
  }
  

  /*********************************************************************
   * getTargetAz() - Get the target azimuth value for this beam.
   */

  double getTargetAzimuth() const
  {
    return _targetAzimuth;
  }
  

  /*********************************************************************
   * getVolumeNum() - Get the volume number for this beam.
   */

  int getVolumeNum() const
  {
    return _volNum;
  }
  

  /*********************************************************************
   * getTiltNum() - Get the tilt number for this beam.
   */

  int getTiltNum() const
  {
    return _tiltNum;
  }
  

protected:
  
  bool _debug;
  bool _verbose;

  ForayUtility::RaycTime _time;
  double _azimuth;
  double _elevation;
  double _targetAzimuth;
  double _targetElevation;
  int _volNum;
  int _tiltNum;
  bool _antennaTransition;
  
  vector< RayDoubles* > _fieldData;
  
};

#endif

