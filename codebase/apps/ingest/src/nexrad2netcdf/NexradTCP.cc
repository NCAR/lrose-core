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
//  Nexrad sub-class for reading from a TCP/IP socket
//
//  Responsible for reading the input data from a TCP/IP socket
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  March 2002
//
//  Note:  Grabbed as is from nexrad2dsr...  Jaimi Yee
//         Sept. 2004
//
//  $Id: NexradTCP.cc,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <dataport/swap.h>
#include <rapformats/swap.h>
#include <toolsa/pmu.h>

#include "Driver.hh"
#include "NexradTCP.hh"
using namespace std;

NexradTCP::NexradTCP()
          :NexradInput()
{
   firstCall = true;
   logicalRecord = NULL;
}

NexradTCP::~NexradTCP()
{
   socket.close();
}

int
NexradTCP::init( Params& params )
{
   //
   // Open the input TCP/IP socket
   //
   if ( socket.open( params.hostname, params.port ) != 0 ) {
      POSTMSG( ERROR, "Unable to open socket %s@%d\n%s",
                       params.hostname, params.port, 
                       socket.getErrString().c_str() );
      return( -1 );
   }

   return( 0 );
}

Status::info_t
NexradTCP::readNexradMsg( ui08* &buffer, bool &volTitleSeen )
{
   Status::info_t  status;
   bool            validData = false;

   //
   // Reset the status of volume title flag
   //
   volumeTitleSeen = false;

   //
   // Pass over any unwanted data records
   //
   while ( !validData ) {

      status = readLogicalRecord();
      switch( status ) {

         case Status::BAD_DATA:
         case Status::END_OF_FILE:
              //
              // Continue onto the next record and/or file
              //
              readLogicalRecord();
              continue;
              break;

         case Status::ALL_OK:
              //
              // Got the good stuff
              //
              validData = true;
              break;

         case Status::BAD_INPUT_STREAM:
         case Status::END_OF_DATA:
         default:
              //
              // Nothing more to be done -- bail out
              //
              return( status );
              break;
      }
   }

   //
   // We've arrived at a valid beam
   // NOTE: the tcp/ip stream never seems to provide a volume title
   //
   buffer = logicalRecord;
   volTitleSeen = volumeTitleSeen;
   return( status );
}

Status::info_t
NexradTCP::readLogicalRecord()
{
   Status::info_t  status;
   PMU_auto_register( "Reading logical record" );

   if ( firstCall ) {
      //
      // Toss the first four bytes (unknown) and a msgHdr
      //
      size_t firstMsgLen = sizeof(RIDDS_msg_hdr) + 4;
      status = readPhysicalRecord( tcpBuffer, firstMsgLen );
      if ( status != Status::ALL_OK ) {
         return( status );
      }
      firstCall = false; 
   }

   //
   // Read the subsequent DIGITAL_RADAR_DATA packet
   //
   status = readPhysicalRecord( tcpBuffer, NEX_BUFFER_SIZE );
   if ( status != Status::ALL_OK ) {
      return( status );
   }

   //
   // See if the packet contains extra info at the front end
   //
   ui08   tmpBuffer[NEX_BUFFER_SIZE];
   // si16  *array = (si16*)tmpBuffer; Not currently used.
   
   memcpy(tmpBuffer, tcpBuffer, NEX_BUFFER_SIZE);
   SWAP_array_16((ui16*)tmpBuffer, NEX_BUFFER_SIZE);
   

   //
   // There is a known word value (1000) in the byte-swapped header
   // that can be used to ascertain the number of EXTRA_BYTE
   // sized packets that are tacked on to the front of
   // this message. This value is at word offset 11. Niles.
   //
   for (int numExtras = 0; numExtras < MAX_EXTRA_PACKETS; numExtras++){
     si16 *testVal = (si16 *) (tmpBuffer + numExtras * EXTRA_BYTES );
     
     if (testVal[11] == 1000){
       //
       // We've got some extra bytes up front so we need to fetch
       // the remaining extra bytes at the end of our buffer.
       //
       status = readPhysicalRecord( tcpBuffer + NEX_BUFFER_SIZE, 
				    numExtras * EXTRA_BYTES );
       if ( status != Status::ALL_OK ) {
	 return( status );
       }
       //
       // Return a pointer to this buffer offset to point to the
       // right place in the buffer.
       //
       logicalRecord = tcpBuffer + numExtras * EXTRA_BYTES;

       //
       // Niles - the following code can be useful if the
       // format changes, causing us to lose synch. It can
       // be useful to see the first few bytes to see where
       // synch has been lost.
       //
       //for (int i=0; i < 16; i++){
       //  cerr << "{" << i << ", " << (int)logicalRecord[i] << "}  ";
       //}
       cerr << endl;
       return Status::ALL_OK;
     }
   }

   //
   // If we got here then we did not find the known word value
   // after MAX_EXTRA_PACKETS tries - we are probably out of synch.
   //
   logicalRecord = NULL;
   return Status::BAD_INPUT_STREAM;

   /* The following commented-out code is from an earlier
   // time when we could assume that there was only one extra
   // packet of 100 bytes. I am leaving it here for future
   // reference since I have mucked with the code but I am
   // not the author - Niles Oien.
   //
   // Check against a known value in the data header
   //
   if ( array[11] == 1000 ) {
      //
      // No extra data, we've got a packet we recognize
      //
      logicalRecord = tcpBuffer;
      return( Status::ALL_OK );
   }

   //
   // We've got some extra bytes up front so we need to fetch
   // the remaining extra bytes at the end of our buffer
   //
   status = readPhysicalRecord( tcpBuffer + NEX_BUFFER_SIZE, EXTRA_BYTES );
   if ( status != Status::ALL_OK ) {
      return( status );
   }
   // End of one block of commented-out code - Niles.
   */



   /* This code was commented out when I started - Niles.
   //
   // The extra bytes up front include a message header
   // See if the record is of a message type that we care about
   //
   ui16           msgType;
   RIDDS_msg_hdr *msgHdr;

   msgHdr = (RIDDS_msg_hdr*)(tcpBuffer + EXTRA_BYTES - sizeof(RIDDS_msg_hdr));
   msgType = ( msgHdr->message_type );

   if ( msgType != DIGITAL_RADAR_DATA ) {
      POSTMSG( DEBUG, "Skipping over message type: %d", msgType );
      return( Status::BAD_DATA );
   }
   */

   /* This code was not commented out when I started - Niles September 2002.
   logicalRecord = tcpBuffer + EXTRA_BYTES;
   return( Status::ALL_OK );
   */
}

Status::info_t
NexradTCP::readPhysicalRecord( ui08 *bufPtr, int nbytes )
{
   PMU_auto_register( "Reading physical record" );

   //
   // Wait until there's something to read from the input socket.
   //  
   if ( socket.readSelectPmu() != 0 ) {
      POSTMSG( ERROR, "Unable to do a readSelect on the socket." );
      return( Status::BAD_INPUT_STREAM );
   } 

   //
   // Read the specified number of bytes
   //  
   if ( socket.readBufferHb( (void*)(bufPtr), nbytes, nbytes,
                            (Socket::heartbeat_t)PMU_auto_register ) != 0 ) {
      POSTMSG( ERROR, "Unsuccessful read from socket." );
      return( Status::BAD_INPUT_STREAM );
   }

   return( Status::ALL_OK );
}
