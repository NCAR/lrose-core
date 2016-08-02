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
//  Nexrad sub-class for reading from TAPE
//
//  Responsible for reading the input data from TAPE
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  Adapted from apps/ridds2mom/tape_input.cc by Gary Blackburn 1991
//
//  $Id: NexradTape.cc,v 1.12 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <dataport/bigend.h>
#include <toolsa/ttape.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>

#include "Driver.hh"
#include "NexradTape.hh"
using namespace std;


NexradTape::NexradTape()
          : NexradInput()
{
   tapeId    = -1;
   bytesLeft = 0;
   firstTime = true;
}

NexradTape::~NexradTape()
{
   if ( tapeId >= 0 ) {
      close( tapeId );
      tapeId = -1;
   }
}

int
NexradTape::init( Params& params )
{
   shortTape = params.short_tape;

   char* tapeDevice = params.input_tape_device;

   if ( strchr( tapeDevice, ':' ) == NULL ) {
      //
      // Open a local tape device
      //
      POSTMSG( DEBUG, "Opening local tape device" );

      remoteDevice = FALSE;
      if (( tapeId = open( tapeDevice, O_RDONLY )) < 0 ) {
         POSTMSG( ERROR, "Unable to open tape device." );
         perror( tapeDevice );
         return( -1 );
      }
      TTAPE_set_var( tapeId );
   } 
   else {
      //
      // Open a remote tape device
      //
      POSTMSG( DEBUG, "Opening remote tape device" );

      remoteDevice = TRUE;
      if ( RMT_open( tapeDevice, O_RDONLY, 0666 ) < 0 ) {
         POSTMSG( ERROR, "Unable to open tape device" );
         perror( tapeDevice );
         return( -1 );
      }
   }

   POSTMSG( DEBUG, "Successfully opened tape device %s", tapeDevice );
   return( 0 );
}

Status::info_t 
NexradTape::readNexradMsg( ui08* &buffer, bool &volTitleSeen )
{
   Status::info_t  status;

   volumeTitleSeen = false;

   //
   // I don't pretend to understand the salient differences between
   // short and non-short tapes.  The logic for distinguishing between
   // these two is retained from Gary Blackburn's original code.
   //
   if ( shortTape ) {
      //
      // Get the next logical record
      //
      status = readLogicalShort();
      if ( status != Status::ALL_OK ) {
         return( status );
      }

      //
      // Set the message pointer for the caller
      //
      buffer = logicalRecord;
   } 
   else {
      //
      // Keep reading until we find the type of record we are looking for
      //
      size_t logRecSize;
      bool found = false;
      char name[9]; char ext[5];
      name[8] = ext[4] = '\0';

      while( !found ) {

         //
         // Get the next logical record
         //
         status = readLogical( logRecSize );
         if ( status != Status::ALL_OK ) {
            return( status );
         }

         //
         // Deduce the type of record based on the record size
         //
         switch( logRecSize ) {

         case sizeof( RIDDS_id_rec ):
              strncpy( name, ((RIDDS_id_rec *)logicalRecord)->filename, 8 );
              POSTMSG( DEBUG, "Skipping over ID message: %s", name );
              break;

         case sizeof( RIDDS_vol_title ):
              volumeTitleSeen = true;
              strncpy( name, ((RIDDS_vol_title *)logicalRecord)->filename, 8 );
              strncpy( ext, ((RIDDS_vol_title *)logicalRecord)->extension, 4 );
              POSTMSG( DEBUG, "Skipping over VOLUME TITLE message: %s.%s", 
                              name, ext );
              break;

         case NEX_PACKET_SIZE:
              //
              // This is the message SIZE we are looking for
              //
              ui08           msgType;
              RIDDS_msg_hdr *msgHdr;

              msgHdr = (RIDDS_msg_hdr*)(logicalRecord + sizeof(RIDDS_ctm_info));
              msgType = msgHdr->message_type;

              if ( msgType != DIGITAL_RADAR_DATA ) {
                 POSTMSG( DEBUG, "Skipping over message type: %u", msgType );
              }
              else {
                 //
                 // This is the message TYPE we are looking for
                 //
                 found = true;
                 size_t skipBytes = sizeof(RIDDS_ctm_info) 
                                  + sizeof(RIDDS_msg_hdr);
                 //
                 // Set the message pointer for the caller
                 //
                 buffer = logicalRecord + skipBytes;
              }
              break;

         default:
              POSTMSG( WARNING, "Unknown tape record size %d", logRecSize );
              break;
         }
      }
   }
    
   volTitleSeen = volumeTitleSeen;
   return( Status::ALL_OK );
}

Status::info_t 
NexradTape::readLogical( size_t& logicalBytes )
{
   //
   // Identifies the beginning of the next logical record
   // within the most recently read physical record (from a non-short tape)
   // Reads in a new physical record when necessary.
   // Upon success returns number of bytes in logical record
   //
   size_t             physicalBytes;
   size_t             logRecSize;
   size_t             counter = 0;
   Status::info_t     status;

   PMU_auto_register( "Reading non-short tape record" );

   if ( bytesLeft == 0 ) {
      //
      // Read in a new physical record from tape
      //
      status = readPhysicalRecord( physicalBytes );

      if ( status != Status::ALL_OK ) {
         return( status );
      }

      if ( physicalBytes == 0 ) {
         if ( firstTime ) {
            //
            // The beginning of nexrad tapes sometimes have 0 sized records
            //
            while ( counter < 5 ) {
	       status = readPhysicalRecord( physicalBytes );
	       if ( status == Status::ALL_OK ) {
	          break;
               }
	       counter++;
	    }
         } 
         else {
            status = readPhysicalRecord( physicalBytes );
            if ( status != Status::ALL_OK ) {
               return( status );
            }
         }
      }
    
      byteOffset = 0;
      bytesLeft = physicalBytes;
   }

   if ( bytesLeft == 0 ) {
      POSTMSG( DEBUG, "End of non-short tape." );
      return( Status::END_OF_DATA );
   }

   else if ( bytesLeft < 0 ) {
      POSTMSG( ERROR, "Tape read error. Bytes read = %d", physicalBytes );
      logicalRecord = (ui08 *)0;
      logRecSize    = bytesLeft;
      return( Status::BAD_INPUT_STREAM );
   } 

   else {
      logicalRecord = physicalRecord + byteOffset;
      if ( firstTime && physicalBytes == sizeof(RIDDS_id_rec) ) {
         logRecSize = physicalBytes;
      } 
      else {
         //
         // Determine which type of record is being read
         //
         if ( strncmp( ((RIDDS_vol_title*)logicalRecord)->filename, 
                       "ARCHIVE2", 8 ) == 0 ) {
            logRecSize = sizeof( RIDDS_vol_title );
         } 
         else {
            logRecSize =  NEX_PACKET_SIZE;
         }
      }

      byteOffset += logRecSize;
      bytesLeft  -= logRecSize;

      if (bytesLeft < logRecSize) {
         bytesLeft = 0;
      }
   }

   firstTime = FALSE;
   logicalBytes = logRecSize;
   return( Status::ALL_OK );	
}

Status::info_t 
NexradTape::readLogicalShort()

{
   //
   // Identifies the beginning of the next logical record
   // within the most recently read physical record (from a short tape).
   // Reads in a new physical record when necessary.
   // Returns number of bytes in logical record, -1 on error.
   //
   size_t             physicalBytes;
   Status::info_t     status;

   PMU_auto_register( "Reading short tape record" );

   if ( bytesLeft < NEX_RAP_TAPE_REC_SIZE ) {
      //
      // Read in a new physical record from tape
      //
      byteOffset = 0;
      status = readPhysicalRecord( physicalBytes );

      if ( status != Status::ALL_OK ) {
         return( status );
      } 

      if ( physicalBytes != NEX_RAP_TAPE_REC_SIZE ) {
         //
         // The physical and logical records are always the same size
         //
         POSTMSG( ERROR, "Unexpected record size for a short tape.\n"
                         "Expected %d bytes -- read %d bytes.", 
                          NEX_RAP_TAPE_REC_SIZE, physicalBytes );
         return( Status::BAD_DATA );
      }

      bytesLeft = physicalBytes;
   }

   //
   // Index into the physical record to establish the logical record
   //
   logicalRecord = physicalRecord + byteOffset;
   byteOffset   += NEX_RAP_TAPE_REC_SIZE;
   bytesLeft    -= NEX_RAP_TAPE_REC_SIZE;

   physicalBytes = NEX_RAP_TAPE_REC_SIZE;	
   return( Status::ALL_OK );	
}

Status::info_t 
NexradTape::readPhysicalRecord( size_t& physicalBytes )
{
   size_t  tries;
   int     bytesRead;

   PMU_auto_register("Reading physical tape record");

   //
   // Read a tape device, giving it 50 chances to succede
   //
   tries = 0;
   while( tries < 50 ) {
      if ( remoteDevice ) {
         bytesRead = readRemoteDevice();
      }
      else {
         bytesRead = readLocalDevice();
      }
      
      if ( bytesRead > 0 ) {
         physicalBytes = bytesRead;
         return( Status::ALL_OK );
      } 
      else {
         //
         // Give it another try
         //
         tries++;
         umsleep(1);
      }
   }

   //
   // If we got this far, the device read did not work
   //
   POSTMSG( ERROR, "Tape device not ready, or no data left\n" );
   return( Status::BAD_INPUT_STREAM );
}

int
NexradTape::readRemoteDevice()
{
   return( RMT_read( (char *)physicalRecord, MAX_REC_LEN ));
}

int
NexradTape::readLocalDevice()
{
   int bytesRead = 0;

   errno = EINTR;
   while( errno == EINTR ) {
      errno = 0;
      bytesRead = read( tapeId, (char *)physicalRecord, MAX_REC_LEN );
   }

   return( bytesRead );
}
