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
//  $Id: NexradTCP.cc,v 1.12 2016/03/07 01:23:10 dixon Exp $
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
  return;
}

NexradTCP::~NexradTCP()
{
   socket.close();
   return;
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

   //
   // Select socket for reading.
   //
   if (socket.readSelectPmu()){
     POSTMSG( ERROR, "Read select failed on socket.");
     return -1;
   }

   return( 0 );
}

Status::info_t
NexradTCP::readNexradMsg( ui08* &buffer, bool &volTitleSeen )
{

  //
  // Enter a loop that we get out of only if we return with
  // a DIGITAL_RADAR_DATA message.
  //
  while(1){

    PMU_auto_register("Reading TCP/IP");

    //
    // Fill up the buffer so we can start looking for a RIDDS_msg_hdr
    // structure.
    //
    
    if (socket.readBuffer(headerBuffer, sizeof(RIDDS_msg_hdr))){
      POSTMSG(ERROR, "Failed to read initial bytes");
      return Status::END_OF_FILE;
    }
    
    //
    // Start looking for a valid ridds header.
    //
    long size = 0L;
    while(true) {

      //
      // Copy the buffer into the byte swapped space.
      //
      ui08 *swappedBuffer = (ui08 *) &riddsHeader;
      for (unsigned i=0; i < sizeof(RIDDS_msg_hdr); i=i+2){
	swappedBuffer[i] = headerBuffer[i+1];
	swappedBuffer[i+1] = headerBuffer[i];
      }

      //
      // Test it to see if holds plausible values.
      //
      
      bool good = true;
      
      long mpm = riddsHeader.millisecs_past_midnight;
      long numSecs = mpm / 1000;
      
      long hour = numSecs/3600;
      numSecs -= hour * 3600;
      
      if ((hour < 0) || (hour > 23)) good=false;
      
      long min = numSecs/60;
      numSecs -= min * 60;
      
      if ((min < 0) || (min > 59) || (numSecs < 0) || (numSecs > 59)) good=false;
      
      size = riddsHeader.message_len;
      if ((size < 50) || (size > 2500)) good = false;

      if (good){
	//
	// We have a good header, break out of this loop.
	//
	POSTMSG(DEBUG, "TCPIP message : size %ld Time %02ld:%02ld:%02ld Type %d",
		size, hour, min, numSecs, riddsHeader.message_type);
	break;
      }

      // No luck - have to keep looking -

      //
      // Shift the buffer, read the next byte. It is tedious to
      // read one byte at a time but hopefully it won't be long
      // before we're in sync.
      //
      for (unsigned i=0; i < sizeof(RIDDS_msg_hdr)-1; i++){
	headerBuffer[i] = headerBuffer[i+1];
      }
      
      if (socket.readBuffer(&headerBuffer[sizeof(RIDDS_msg_hdr)-1], 1)){
	POSTMSG( ERROR, "Failed to read extra byte.");
	return Status::END_OF_FILE;
      }

    } // End of eternal loop that we only exit with the above 'break' statement.

    //
    // If we got to here, then the variable 'size' holds the number of bytes
    // to read, and we've just finished reading the ridds header. The
    // messages seem to have a 4 byte CRC check (?) attached so skip
    // that too.
    //
    long numToRead = size + 4 - sizeof(RIDDS_msg_hdr);

    if (socket.readBuffer(digitalRadarMessageBuffer, numToRead)){
      POSTMSG(ERROR, "Failed to read message entry.");
      return Status::END_OF_FILE;
    }
    
    //
    // We need to offset by two bytes. I'm not entirely sure
    // why this is - I think perhaps the four extra bytes we read
    // are two sets of two CRC check bytes, one after the header and one
    // after the radar packet.
    //
    // This was ascertained with the knowldege there there is a
    // known word in the byte-swapped header - the two bytes at word
    // offset 11 sould equal the value 1000. A Handy piece of info for those
    // trying to decode this stream!
    //
    // In fact, we will only return if this header value checks out. Otherwise we will
    // stick with the eternal loop - skipping this message (which probably is not
    // a DIGITAL_RADAR_DATA message).
    
    if (1000 == 256*digitalRadarMessageBuffer[24] + digitalRadarMessageBuffer[25]){

      buffer = digitalRadarMessageBuffer + 2; // Extra two bytes skips CRC ?
      volTitleSeen = false;

      return Status::ALL_OK;

    }

  } // End of eternal loop.

}


