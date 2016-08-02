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
////////////////////////////////////////////////////////////////////////////////
                                 
#include <cassert>
#include <iostream>

#include <dataport/bigend.h>                    
#include <Fmq/DsRadarQueue.hh>                    
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
using namespace std;
   
DsRadarQueue::DsRadarQueue()
             :DsFmq()
{
}

/////////////////////////////////////////////////////////////
// getDsMsg()
//
// Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents
// Blocking read.
//

int
DsRadarQueue::getDsMsg( DsRadarMsg &dsRadarMsg, int *content )
{
   PMU_auto_register("In DsRadarQueue::getDsBeam");
   
   if ( readMsgBlocking( DS_MESSAGE_TYPE_DSRADAR ) ||
        getMsgLen() == 0 ) {
      return( -1 );
   }

   //                               
   // Decode the radar message from the standard bigEndian format
   //
   int thisMsgContent;
   int iret = dsRadarMsg.disassemble( getMsg(), getMsgLen(), &thisMsgContent );
   *content = thisMsgContent;

   return (iret);
}

// Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents
// Non-blocking read.
// Sets gotOne to TRUE if a message was read.

int
DsRadarQueue::getDsMsg( DsRadarMsg &dsRadarMsg, int *content, bool *gotOne )
{
   PMU_auto_register("In DsRadarQueue::getDsBeam");

   if ( readMsg( gotOne, DS_MESSAGE_TYPE_DSRADAR ) ) {
      return( -1 );
   }

   if ( !*gotOne ) {
      return( 0 );
   }

   if ( getMsgLen() == 0 ) {
      return( -1 );
   }

   //                               
   // Decode the radar message from the standard bigEndian format
   //
   int thisMsgContent;
   int iret = dsRadarMsg.disassemble( getMsg(), getMsgLen(), &thisMsgContent );
   *content = thisMsgContent;

   return (iret);
}

///////////////////////////////////////////////////////////////////
// getDsBeam()
//
// Reads a DS_MESSAGE_TYPE_DSRADAR message with any contents except
//  the radar flags
//

int
DsRadarQueue::getDsBeam( DsRadarMsg &dsRadarMsg, int *content )
{

   int forever = TRUE;

   while (forever) {

     if (getDsMsg( dsRadarMsg, content)) {
       return (-1);
     }

     if ((*content & DsRadarMsg::RADAR_BEAM)) {
       return (0);
     }

   }

   return (-1);

}

////////////////////////////////////////////////////////////
// putDsMsg()
//
// Puts a DS_MESSAGE_TYPE_DSRADAR message with any contents
//

int
DsRadarQueue::putDsMsg( DsRadarMsg &dsRadarMsg, int content )
{

  ui08 *msg;
  
  //
  // Encode the radar message into standard bigEndian format
  //
  msg =  dsRadarMsg.assemble( content );
  int msgLength = dsRadarMsg.lengthAssembled();
  
  if (msgLength > 0) {
    if (writeMsg( DS_MESSAGE_TYPE_DSRADAR, 0, msg, msgLength )) {
      return( -1 );
    }
  } else {
    return (-1);
  }

  return (0);

}

//////////////////////////////////////////////////////////////////
// putDsBeam()
//
// Puts a DS_MESSAGE_TYPE_DSRADAR message with any contents except
// flag type
//

int
DsRadarQueue::putDsBeam( DsRadarMsg &dsRadarMsg, int content )
{

  if (content & DsRadarMsg::RADAR_FLAGS) {
    fprintf(stderr, "ERROR - trying to put beam with flags content\n");
    return ( -1 );
  }
  
  return ( putDsMsg(dsRadarMsg, content));

}

////////////////////////////////////
// putStartOfTilt()
//
// Puts a startOfTilt flags message
//

int
DsRadarQueue::putStartOfTilt( int tiltNum, time_t time )
{

   DsRadarFlags& flags = _dsRadarMsg.radarFlags;
   flags.clear();
   flags.time = time;
   flags.tiltNum = tiltNum;
   flags.startOfTilt = TRUE;
   int content = DsRadarMsg::RADAR_FLAGS;

   return (putDsMsg(_dsRadarMsg, content));

}

////////////////////////////////////
// putEndOfTilt()
//
// Puts a endOfTilt flags message
//

int
DsRadarQueue::putEndOfTilt( int tiltNum, time_t time )
{

   DsRadarFlags& flags = _dsRadarMsg.radarFlags;
   flags.clear();
   flags.time = time;
   flags.tiltNum = tiltNum;
   flags.endOfTilt = TRUE;
   int content = DsRadarMsg::RADAR_FLAGS;

   return (putDsMsg(_dsRadarMsg, content));

}

////////////////////////////////////
// putStartOfVolume()
//
// Puts a startOfVolume flags message
//

int
DsRadarQueue::putStartOfVolume( int volumeNum, time_t time )
{

   DsRadarFlags& flags = _dsRadarMsg.radarFlags;
   flags.clear();
   flags.time = time;
   flags.volumeNum = volumeNum;
   flags.startOfVolume = TRUE;
   int content = DsRadarMsg::RADAR_FLAGS;

   return (putDsMsg(_dsRadarMsg, content));

}

////////////////////////////////////
// putEndOfVolume()
//
// Puts a endOfVolume flags message
//

int
DsRadarQueue::putEndOfVolume( int volumeNum, time_t time )
{

   DsRadarFlags& flags = _dsRadarMsg.radarFlags;
   flags.clear();
   flags.time = time;
   flags.volumeNum = volumeNum;
   flags.endOfVolume = TRUE;
   int content = DsRadarMsg::RADAR_FLAGS;

   return (putDsMsg(_dsRadarMsg, content));

}

////////////////////////////////////
// putNewScanType()
//
// Puts a newScanType flags message
//

int
DsRadarQueue::putNewScanType( int scanType, time_t time )
{

   DsRadarFlags& flags = _dsRadarMsg.radarFlags;
   flags.clear();
   flags.time = time;
   flags.scanType = scanType;
   int content = DsRadarMsg::RADAR_FLAGS;

   return (putDsMsg(_dsRadarMsg, content));

}

/////////////////////////////////////////////////////////////////
// isStartOfTilt()
//
// Reads until a startOfTilt flags message with the correct tilt
// number is received. If the requested tilt is -1, any tilt is
// accepted.
//

bool
DsRadarQueue::isStartOfTilt( int tilt, time_t *when )
{
  bool status;
  int forever = TRUE;
   
  while (forever) {
    
    int content;
    
    if (getDsMsg( _dsRadarMsg, &content)) {
      return false;
    }

    if (!(content & DsRadarMsg::RADAR_FLAGS)) {
      continue;
    }
    
    DsRadarFlags& flags = _dsRadarMsg.radarFlags;
    
    if (!flags.startOfTilt) {
      continue;
    }
    
    if ( tilt == -1 ) {
      //
      // Any ol' tilt will do
      //
      status = true;
    }
    else {
      //
      // Check for a specific volume number
      //
      if ( tilt == flags.tiltNum )
	status = true;
      else
	status = false;
    }
    
    //
    // Return the data time, if requested
    //
    if ( when != NULL ) {
      *when = (time_t) flags.time;
    }
    return( status );

  } // while

  return ( false );

}

/////////////////////////////////////////////////////////////////
// isEndOfTilt()
//
// Reads until a endOfTilt flags message with the correct tilt
// number is received. If the requested tilt is -1, any tilt is
// accepted.
//

bool  
DsRadarQueue::isEndOfTilt( int tilt, time_t *when )
{
  bool status;
  int forever = TRUE;
   
  while (forever) {
    
    int content;
    
    if (getDsMsg( _dsRadarMsg, &content)) {
      return false;
    }

    if (!(content & DsRadarMsg::RADAR_FLAGS)) {
      continue;
    }
    
    DsRadarFlags& flags = _dsRadarMsg.radarFlags;
    
    if (!flags.endOfTilt) {
      continue;
    }
    
    if ( tilt == -1 ) {
      //
      // Any ol' tilt will do
      //
      status = true;
    }
    else {
      //
      // Check for a specific volume number
      //
      if ( tilt == flags.tiltNum )
	status = true;
      else
	status = false;
    }
    
    //
    // Return the data time, if requested
    //
    if ( when != NULL ) {
      *when = (time_t) flags.time;
    }
    return( status );

  } // while

  return ( false );

}

/////////////////////////////////////////////////////////////////
// isStartOfVolume()
//
// Reads until a startOfVolume flags message with the correct volume
// number is received. If the requested volume is -1, any volume is
// accepted.
//

bool  
DsRadarQueue::isStartOfVolume( int volume, time_t *when )
{
  bool status;
  int forever = TRUE;
   
  while (forever) {
    
    int content;
    
    if (getDsMsg( _dsRadarMsg, &content)) {
      return false;
    }

    if (!(content & DsRadarMsg::RADAR_FLAGS)) {
      continue;
    }
    
    DsRadarFlags& flags = _dsRadarMsg.radarFlags;
    
    if (!flags.startOfVolume) {
      continue;
    }
    
    if ( volume == -1 ) {
      //
      // Any ol' volume will do
      //
      status = true;
    }
    else {
      //
      // Check for a specific volume number
      //
      if ( volume == flags.volumeNum )
	status = true;
      else
	status = false;
    }
    
    //
    // Return the data time, if requested
    //
    if ( when != NULL ) {
      *when = (time_t) flags.time;
    }
    return( status );

  } // while

  return ( false );

}

/////////////////////////////////////////////////////////////////
// isEndOfVolume()
//
// Reads until a endOfVolume flags message with the correct volume
// number is received. If the requested volume is -1, any volume is
// accepted.
//

bool  
DsRadarQueue::isEndOfVolume( int volume, time_t *when )
{
  bool status;
  int forever = TRUE;
   
  while (forever) {
    
    int content;
    
    if (getDsMsg( _dsRadarMsg, &content)) {
      return false;
    }

    if (!(content & DsRadarMsg::RADAR_FLAGS)) {
      continue;
    }
    
    DsRadarFlags& flags = _dsRadarMsg.radarFlags;
    
    if (!flags.endOfVolume) {
      continue;
    }
    
    if ( volume == -1 ) {
      //
      // Any ol' volume will do
      //
      status = true;
    }
    else {
      //
      // Check for a specific volume number
      //
      if ( volume == flags.volumeNum )
	status = true;
      else
	status = false;
    }
    
    //
    // Return the data time, if requested
    //
    if ( when != NULL ) {
      *when = (time_t) flags.time;
    }
    return( status );

  } // while

  return ( false );

}

/////////////////////////////////////////////////////////////////
// isNewScanType()
//
// Reads until a newScanType flags message with the correct scanType
// number is received. If the requested scanType is -1, any scanType is
// accepted.
//

bool  
DsRadarQueue::isNewScanType( int scanType, time_t *when )
{
  bool status;
  int forever = TRUE;
   
  while (forever) {
    
    int content;
    
    if (getDsMsg( _dsRadarMsg, &content)) {
      return false;
    }

    if (!(content & DsRadarMsg::RADAR_FLAGS)) {
      continue;
    }
    
    DsRadarFlags& flags = _dsRadarMsg.radarFlags;
    
    if (!flags.newScanType) {
      continue;
    }
    
    if ( scanType == -1 ) {
      //
      // Any ol' scan type will do
      //
      status = true;
    }
    else {
      //
      // Check for a specific scan type
      //
      if ( scanType == flags.scanType )
	status = true;
      else
	status = false;
    }
    
    //
    // Return the data time, if requested
    //
    if ( when != NULL ) {
      *when = (time_t) flags.time;
    }
    return( status );

  } // while

  return ( false );

}

