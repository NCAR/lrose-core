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
 * @file ElevVolumeTrigger
 * @brief Class for determining the end of volume based on
 *        the change in elevation angle between sweeps
 * @class ElevVolumeTrigger
 * @brief Class for determining the end of volume based on
 *        the change in elevation angle between sweeps
 */

#ifndef ElevVolumeTrigger_HH
#define ElevVolumeTrigger_HH

#include "VolumeTrigger.hh"

class ElevVolumeTrigger : public VolumeTrigger
{
 public:

  /**
   * Constructor
   */
  ElevVolumeTrigger();
  

  /**
   * Destructor
   */
  virtual ~ElevVolumeTrigger(void);
  

  /**
   * Determines if we are starting to process a new volume.
   *
   * @Return true if this is a new volume, false otherwise.
   */

  virtual bool isNewVolume(const SweepFile &sweep_file);


 protected:

  double _currElevAngle;  /**< Most recent elevation angle */

};


#endif
