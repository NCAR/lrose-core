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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:13 $
//   $Id: StartTimeVolumeTrigger.cc,v 1.2 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * StartTimeVolumeTrigger: Class for determining the end of volume based on
 *                         the value of the volume start time field in the
 *                         netCDF sweep file.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "StartTimeVolumeTrigger.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

StartTimeVolumeTrigger::StartTimeVolumeTrigger(const bool debug_flag) :
  VolumeTrigger(debug_flag),
  _currVolumeStartTime(DateTime::NEVER)
{
}


/*********************************************************************
 * Destructor
 */

StartTimeVolumeTrigger::~StartTimeVolumeTrigger()
{
}


/*********************************************************************
 * isNewVolume() - Determines if we are starting to process a new volume.
 *
 * Returns true if this is a new volume, false otherwise.
 */

bool StartTimeVolumeTrigger::isNewVolume(const SweepFile &sweep_file)
{
  bool is_new_volume = false;
  DateTime volume_start_time;
  
  if (!sweep_file.getVolumeStartTime(volume_start_time))
    return false;
  
  if (_currVolumeStartTime == DateTime::NEVER)
  {
    _currVolumeStartTime = volume_start_time;
    is_new_volume = false;
  }
  else if (_currVolumeStartTime != volume_start_time)
  {
    is_new_volume = true;
    _currVolumeStartTime = volume_start_time;
  }
  
  return is_new_volume;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
