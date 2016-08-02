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
////////////////////////////////////////////////////////////////////////////////
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  February 1999
//
// $Id: DsRadarQueue.hh,v 1.10 2016/03/03 18:56:47 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_RADAR_QUEUE_INC_
#define _DS_RADAR_QUEUE_INC_

#include <dataport/port_types.h>      
#include <didss/ds_message.h>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsFmq.hh>
using namespace std;

class DsRadarQueue : public DsFmq
{
public:

  DsRadarQueue();
  virtual ~DsRadarQueue(){};
  
  // Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents
  // Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents
  // Blocking read.
  // Returns 0 on success, -1 on error
  
  int getDsMsg( DsRadarMsg &newRadarInfo, int *content=NULL );
  
  // Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents
  // Non-blocking read.
  // Returns 0 on success, -1 on error
  // Sets gotOne to TRUE if a message was read.

  int getDsMsg( DsRadarMsg &dsRadarMsg, int *content, bool *gotOne );

  int          putDsMsg( DsRadarMsg &newRadarInfo, int content=0 );
  
  int          getDsBeam( DsRadarMsg &newRadarInfo, int *content=NULL );
  int          putDsBeam( DsRadarMsg &newRadarInfo, int content=0 );
  
  int          putStartOfTilt( int tiltNum, time_t time = -1 );
  int          putStartOfVolume( int volumeNum, time_t time = -1 );
  int          putEndOfTilt( int tiltNum, time_t time = -1 );
  int          putEndOfVolume( int volumeNum, time_t time = -1 );
  int          putNewScanType( int scanType, time_t time = -1 );
  
  bool         isStartOfTilt( int tilt = -1, time_t *when = NULL );
  bool         isStartOfVolume( int volume = -1, time_t *when = NULL );
  bool         isEndOfTilt( int tilt = -1, time_t *when = NULL );
  bool         isEndOfVolume( int volume = -1, time_t *when = NULL );
  bool         isNewScanType( int scanType = -1, time_t *when = NULL );

private:

  DsRadarMsg _dsRadarMsg; // used for flag messages

};

#endif
