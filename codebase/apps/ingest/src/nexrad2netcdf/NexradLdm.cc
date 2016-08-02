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
/////////////////////////////////////////////////////////////////////////
//  Nexrad sub-class for reading from an LDM radar stream
//
//  Responsible for reading the input data from a LDM file
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  Note:  Grabbed as is from nexrad2dsr...  Jaimi Yee
//         Sept. 2004
//
//  $Id: NexradLdm.cc,v 1.25 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/utim.h>

#include <bzlib.h>

#include "Driver.hh"
#include "NexradLdm.hh"
using namespace std;

//
// Constants
//
const int NexradLdm::FILE_NAME_LEN = 100;

NexradLdm::NexradLdm()
          :NexradInput()
{
   bytesLeft               = 0;
   byteOffset              = 0;
   logicalRecord           = NULL;
   fileTrigger             = NULL;
   dsTrigger               = NULL;
   ldmFile                 = NULL;
   stateFileName           = false;
   fileIsOpen              = false;
   realtimeMode            = false;
   realtimeSequenceNumber  = -1;
   realtimeDatatime        = 0;
   radarInputDir           = NULL;
   datedDirFormat          = NULL;
   fileTimeFormat          = NULL;
   maxValidAgeMin          = -1;
   maxElapsedSearchTimeMin = -1;
   Qtime                   = -1;
   minFileSize             = -1;
   isBuild5                = false;
   noVolumeTitleYet        = true;
   oneFilePerVolume        = false;
   compression             = UNCOMPRESSED;
   msgCount                = 0;
}

NexradLdm::~NexradLdm()
{
   delete fileTrigger;
   delete dsTrigger;
   delete stateFileName;

   if ( ldmFile ) {
      fclose( ldmFile );
   }

   free( radarInputDir );
   free( datedDirFormat );
   free( fileTimeFormat );

}

int
NexradLdm::init( Params& params )
{
   //
   // Set up input data file triggering
   //
   stateFileName          = new string( params.state_file_name );
   usePrevState            = ( params.use_previous_state ) ? true : false;
   radarInputDir           = strdup( params.radar_input_dir );
   datedDirFormat          = strdup( params.dated_dir_format );
   fileTimeFormat          = strdup( params.file_time_format );
   maxValidAgeMin          = params.max_valid_age_min;
   maxElapsedSearchTimeMin = params.max_elapsed_search_time;
   Qtime                   = params.file_quiescence_sec;
   minFileSize             = params.min_file_size;
   isBuild5                = params.build5;
   oneFilePerVolume        = params.oneFilePerVolume;
   instance                = params.instance;

   if ( params.mode == Params::REALTIME ) {
      realtimeMode = true;
      POSTMSG( DEBUG, "Initializing realtime input from '%s'.",
                       params.radar_input_dir );
      readState();
   }
   else {
      realtimeMode = false;
      const vector <string> & inputFileList = driver->getInputFileList();
      if ( inputFileList.size() ) {
         POSTMSG( DEBUG, "Initializing archive file list input" );
         //
         // For now we are switching to DsTrigger for archive processing
         //
         DsFileListTrigger* fileTrigger = new DsFileListTrigger();
         fileTrigger->init( inputFileList );
         dsTrigger = fileTrigger;
      }
      else {
         POSTMSG( ERROR, "Start/end time options currently disabled" );
      }

   }
                        
   //
   // Compression
   //
   switch( params.ldm_compression ) {
      case Params::UNCOMPRESSED:
           compression = UNCOMPRESSED;
           break;
      case Params::BZIP2:
           compression = BZIP2;
           break;
      case Params::ZLIB:
           compression = ZLIB;
           break;
   }

   //
   // Set up temporary path for decompressed data
   //
   tmpPath = params.tmp_dir;
   tmpPath += "/nexrad2netcdf.uncompressed.data.";
   tmpPath += params.instance;
   return( 0 );
}

Status::info_t
NexradLdm::readNexradMsg( ui08* &buffer, bool &volTitleSeen )
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
              // CONTINUE onto the next record and/or file
              //
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
   //
   buffer = logicalRecord;
   volTitleSeen = volumeTitleSeen;
   msgCount++;
   return( status );
}

Status::info_t
NexradLdm::readLogicalRecord()
{
   //
   // Identifies the beginning of the next logical record
   // within the most recently read physical record.
   // Reads in a new physical record when necessary.
   //
   size_t             physicalBytes;
   Status::info_t     status;

   PMU_auto_register( "Reading logical record" );

   //
   // See if we had an unusual record size on the last go 'round
   //
   if ( bytesLeft < 0 ) {
      POSTMSG( ERROR, "Logical read error. Unexpected bytes left = %d", 
                      bytesLeft );
      return( Status::BAD_INPUT_STREAM );
   } 

   //
   // See if we need to read in a new physical record
   //
   if ( bytesLeft == 0 ) {
      status = readPhysicalRecord( physicalBytes );

      if ( status != Status::ALL_OK ) {
         return( status );
      }

      byteOffset = 0;
      bytesLeft = physicalBytes;
   }

   //
   // Move to the next logical record
   //
   logicalRecord = ldmBuffer + byteOffset;
   byteOffset   += NEX_PACKET_SIZE;
   bytesLeft    -= NEX_PACKET_SIZE;

   //
   // See if the record is of a message type that we care about
   //
   si16           msgType;
   RIDDS_msg_hdr *msgHdr;

   msgHdr = (RIDDS_msg_hdr*)(logicalRecord + sizeof(RIDDS_ctm_info));
   msgType = ( msgHdr->message_type );

   if ( msgType != DIGITAL_RADAR_DATA ) {
      POSTMSG( DEBUG, "Skipping over message type: %d", msgType );
      return( Status::BAD_DATA );
   }

   //
   // This is the message TYPE we are looking for, i.e., DIGITAL_RADAR_DATA
   // Jump beyond the message headers and call it a success
   //
   logicalRecord += sizeof(RIDDS_ctm_info) + sizeof(RIDDS_msg_hdr);

   return( Status::ALL_OK );
}

Status::info_t
NexradLdm::readPhysicalRecord( size_t& physicalBytes )
{
   int             bytesRead;
   Status::info_t  status;

   PMU_auto_register("Reading physical record");

   //
   // See if we need to open a new file
   //
   if ( !fileIsOpen ) {

      POSTMSG( DEBUG, "%d messages in file %s\n", msgCount, fileNameBuffer );
      msgCount = 0;

      strncpy( prevFilePath, fileNameBuffer, MAX_PATH_LEN );

      status = openNextFile();
      if ( status != Status::ALL_OK ) {
         return( status );
      }

      //
      // See if we have a volume title record that we need to skip over
      //
      bytesRead = fread( (void*)ldmBuffer, sizeof(ui08),
                         sizeof( RIDDS_vol_title ), ldmFile );

      if ( bytesRead == 0 ) {
         //
         // Oops! Can't even read the first 24 bytes from a newly opened file 
         //
         POSTMSG( ERROR, "Unable to read from input file %s",
                  fileNameBuffer );
         closeLdmFile();
         physicalBytes = 0;
         return( Status::BAD_INPUT_STREAM );
      }

      char *fname = ((RIDDS_vol_title *) ldmBuffer)->filename;
      if ( (strncmp(fname, "ARCHIVE2", 8 )) &&
	   (strncmp(fname, "AR2V", 4 )) ) {

         //
         // Nope, not a volume title, rewind to the beginning of the file
         // so we can read in the entire nexrad record
         //
         rewind( ldmFile );
      }
      else {

         //
         // Yep, we found a volume title at the beginning of the new file.
 	 //
	 noVolumeTitleYet = false;

	 //
	 // If this is a build 5 file and we are not in oneFilePerVolume
	 // mode, then skip this file since it contains only metadata.
	 //
	 if ( (isBuild5) && (!(oneFilePerVolume)) ) {
	   closeLdmFile();
	   physicalBytes = 0;
	   return( Status::END_OF_FILE );
	 }

         //
         // Make note of the fact that we hit a volume title boundary
         // This information will be used as an additional check for
         // startOfVolume since the nexrad data stream seems to be
         // a bit remiss in setting the radial_status flags.
         //
         volumeTitleSeen = true;

         if ( DEBUG_ENABLED ) {
            char name[9];
            char ext[5];
            name[8] = ext[4] = '\0';

            strncpy( name, ((RIDDS_vol_title *)ldmBuffer)->filename, 8 );
            strncpy( ext, ((RIDDS_vol_title *)ldmBuffer)->extension, 4 );
            POSTMSG( DEBUG, "Volume title: %s.%s", name, ext );

	    if ( isBuild5 ) {
	       POSTMSG( DEBUG, "Assuming build 5 data." );
	    } else {
	       POSTMSG( DEBUG, "Assuming data are not build 5." );
	    }
         }
         
         //
	 // If this is build 5 data, we have to skip the
	 // metadata that immediately follows the volume
	 // header.
	 //
	 if ( isBuild5 ) {
            ui32 sizeRecordHeader;
	    int  numRead = fread( &sizeRecordHeader, sizeof(ui32), 
                                  1, ldmFile );

	    if (numRead != 1){
               perror("Failed to get metadata size.");
	       return( Status::BAD_INPUT_STREAM );
	    }

	    _byteSwap4( &sizeRecordHeader );

	    if ( 0 != fseek(ldmFile, sizeRecordHeader, SEEK_CUR) ) {
               perror( "Failed to skip metadata." );
	       return( Status::BAD_INPUT_STREAM );
            }

	    closeLdmFile();
	    physicalBytes = 0;
	    return( Status::END_OF_FILE );
         }
      }
   }

   //
   // Get a block of 100 NEXRAD records from the file.
   // Again, this depends if the data are build 5 or not, and if
   // they are delivered as one file per volume.
   //
   if ( (isBuild5) && (oneFilePerVolume) ) {
     si32 sizeRecordHeader;
     int numRead = fread( &sizeRecordHeader, sizeof(si32), 1, ldmFile );
   
     if ( numRead != 1 ) {
        perror( "Failed to get size for 100 beam data." );
        return( Status::BAD_INPUT_STREAM );
     }
   
     _byteSwap4( &sizeRecordHeader );

     if (sizeRecordHeader < 1 ) {

        //
        // Time to close the file.
        //
        closeLdmFile();
        physicalBytes = 0;
        return( Status::END_OF_FILE );
     }

     //
     // Read the compressed data into a malloc'ed buffer, and
     // decompress into the usual place.
     //
     char *inCompressedData = (char *) malloc(sizeRecordHeader);

     if ( inCompressedData == NULL ) {
        POSTMSG( ERROR, "Malloc failed." );
        exit(-1);
     }

     numRead = fread( inCompressedData, sizeof(ui08), 
                      sizeRecordHeader, ldmFile );

     if ( numRead != sizeRecordHeader ) {
        perror( "Failed to get 100 beam data." );
        return( Status::BAD_INPUT_STREAM );
     }

     //
     // Decompress these data.
     //
     unsigned int numOut = NEX_BUFFER_SIZE;
     int r = BZ2_bzBuffToBuffDecompress ( (char *)ldmBuffer,
					  &numOut,
					  inCompressedData,
					  sizeRecordHeader,
					  0,0 );

     if ( r != BZ_OK ) {
        fprintf( stderr,"BZ failure.\n" );
        exit(-1);
     }

     free ( inCompressedData );
     bytesRead = numOut;

   } 
   else {

      //
      // These are not build 5 data - they are not compressed.
      // Just read them in.
      //
      bytesRead = fread( (void*)ldmBuffer, sizeof(ui08), 
                         NEX_BUFFER_SIZE, ldmFile );

   }


   if ( bytesRead == 0 ) {

      //
      // The next read of a physical record will force a new file open
      //
      closeLdmFile();
      physicalBytes = 0;
      return( Status::END_OF_FILE );
   }

   physicalBytes = bytesRead;
   return( Status::ALL_OK );
}

void
NexradLdm::closeLdmFile()
{
   fclose( ldmFile );

   fileIsOpen  = false;
   ldmFile     = NULL;
}


Status::info_t
NexradLdm::openNextFile()
{
   char *filePath;
   
   POSTMSG( DEBUG, "Fetching next input data file..." );

   //
   // Get the next file path, skipping over any directories
   //
   if ( realtimeMode ) {

     //
     // Realtime mode - need to do some minor string
     // searching to make sure we get the file in the right order.
     //
     do {
       filePath = getNextLdmRealtimeFile();
     } while (filePath == NULL);
   }
   else {

     if ( fileTrigger ) {

       //
       // Archive mode - can just trust the order of the files.
       //
       filePath = fileTrigger->next();
       
       if ( !filePath ) {
	 POSTMSG( DEBUG, "Completed processing all input data files." );
	 return( Status::END_OF_DATA );
       }
     }
     else {
       if ( !dsTrigger ) {
	 POSTMSG( DEBUG, "dsTrigger unititalized - exiting." );
	 exit(-1);
       }

       TriggerInfo triggerInfo;
       if( dsTrigger->next( triggerInfo ) == -1 ) {
          POSTMSG( DEBUG, "Completed processing all input data files." );
          return( Status::END_OF_DATA );
       }

       filePath = (char*)triggerInfo.getFilePath().c_str();
     }
   }

   POSTMSG( DEBUG, "Attempting to open file '%s'", filePath );

   //
   // Uncompress the input file, if necessary
   // For now we will make a system call to the ldm utility
   // Eventually, it would be much better to have that code compiled in
   //

   //
   // If we are doing non-volume build 5 files, we
   // don't decompress if realtimeSequenceNumber
   // is one - the metadata does not decompress.
   //
   bool decompress = true;
   if ( compression == UNCOMPRESSED ) {
      decompress = false;
   }
   if ( (realtimeSequenceNumber == 1) && (isBuild5) && 
        (!(oneFilePerVolume)) ) {
      decompress = false;
   }

   if ( decompress ) {
      char   command[2048];

      //
      // Remove the temporary file from before
      //
      unlink( tmpPath.c_str() );

      if ( compression != BZIP2 ) {
         POSTMSG( ERROR, "BZIP2 is the only compression type supported" );
         return( Status::FAILURE );
      }

      time_t now = time( NULL );
      POSTMSG( DEBUG, "Uncompressing input file at %s", UTIMstr(now) );
 
      if( DEBUG_ENABLED ) {
         sprintf( command, "nexradBzUncomp -if %s -of %s -debug -instance %s",
                  filePath, tmpPath.c_str(), instance.c_str() );
      }
      else {
        sprintf( command, "nexradBzUncomp -if %s -of %s -instance %s",
                 filePath, tmpPath.c_str(), instance.c_str() );
      }

      POSTMSG( DEBUG, "command = %s", command );
      system( command );

      /*-----------------------------------------------------------
      Currently, the return code from nexradII_bz is something like
      15872 which represents I don't know what.  So for now we will
      assume the uncompression works without checking a return code.
      int rc = system( command );
      if ( rc != 0 ) {
         POSTMSG( ERROR, "Return code %d from decompression", rc );
         return( Status::BAD_INPUT_STREAM );
      }
      ------------------------------------------------------------*/

      //
      // Make the temporary file the new file path
      //
      filePath = (char*)tmpPath.c_str();
   }

   //
   // Open the file readOnly
   //
   ldmFile = fopen( filePath, "r" );
   if ( !ldmFile ) {
      POSTMSG( ERROR, "Unable to open file '%s'", filePath );
      perror( filePath );
      return( Status::BAD_INPUT_STREAM );
   }

   //
   // Successfully opened the file
   //
   fileIsOpen = true;
   return( Status::ALL_OK );
}




//
// Returns either the next realtime file or NULL if things did
// not go well. If NULL is returned, the search parameters have been
// reset (ie. we start looking again with a clean slate, not trying to follow
// a sequence).
//
char* NexradLdm::getNextLdmRealtimeFile()
{
  time_t searchStartTime = time(NULL);
   
  int elapsedSearchTime = -1;

  /////////////////////////////////////////////////
  // MAKE ONE DO LOOP WITH THE IF STATEMENT INSIDE
  ////////////////////////////////////////////////  
  if (realtimeSequenceNumber < 0) {

    //
    // OK - need something with sequence number 0 and
    // a data time greater than the last data time. In this
    // case we need to look at the next dated data dir, too,
    // because we may have rolled over into the next day.
    //
    bool loop      = true;
    int  loopCount = 0;

    char file[MAX_PATH_LEN];

    do {
      char dir[MAX_PATH_LEN];

      getDir( dir, false );

      if ( getFile(dir, file) ) {

	//
	// If that failed, check the next directory.
	//
	getDir( dir, true );

	if ( !(getFile(dir, file)) ) {

           //
           // If we got a file, stop looping
           //
	  loop = false;
	}

      } else {

         //
         // If we got a file, stop looping
         //
	loop = false;

      }

      //
      // Always sleep for a second - it avoids thrashing
      // the filesystem, and allows the file time to be written.
      //
      sleep(1);
      loopCount++;
      if (loopCount % 5 == 0){
	PMU_auto_register("Looking for start of new volume");
      }

      //
      // Decide if we've taken too long looking for a file
      //
      elapsedSearchTime = time(NULL) - searchStartTime;
      if (elapsedSearchTime > 60*maxElapsedSearchTimeMin){
	POSTMSG( WARNING, "max search time elapsed\n" );
	resetFileSearchParameters();
	return NULL;
      }

    } while ( loop );

    POSTMSG( DEBUG, "Done looking for file" );

    sprintf(fileNameBuffer, "%s", file);

  } 
  else {

    //
    // Know pretty much what we are looking for - a file with
    // exactly the same data time and an incremented sequence
    // number (Or the letter 'E' indicating the end of the sequence).
    // In this case there is no need to look at the next dated dir.
    //
    bool loop      = true;
    int  loopCount = 0;

    char file[MAX_PATH_LEN];

    do {
      char dir[MAX_PATH_LEN];

      getDir( dir, false );

      if ( !(getFile(dir, file)) ) {

         //
         // If we got a file, stop looping
         //
	 loop = false;
      }

      //
      // Always sleep for a second - it avoids thrashing
      // the filesystem, and allows the file time to be written.
      //
      sleep(1);

      loopCount++;
      if (loopCount % 5 == 0){
	PMU_auto_register("Looking for new file");
      }

      //
      // See if we've spent too much time looking for a file
      //
      elapsedSearchTime = time(NULL) - searchStartTime;
      if (elapsedSearchTime > 60*maxElapsedSearchTimeMin){
	resetFileSearchParameters();
	return NULL;
      }

    } while (loop);
      
    POSTMSG( DEBUG, "Done looking for file" );

    sprintf(fileNameBuffer, "%s", file);

  }

  realtimeDatatime = parseRealtimeLdmFilename( fileNameBuffer, 
                                               realtimeSequenceNumber );

  POSTMSG( DEBUG, "Found file : %s", fileNameBuffer );
  POSTMSG( DEBUG, "The time has been set to %s", utimstr( realtimeDatatime ));
  POSTMSG( DEBUG, "The sequence number is %d",realtimeSequenceNumber );
  POSTMSG( DEBUG, "Found in %d seconds", elapsedSearchTime );

  return fileNameBuffer;

}

// 
// This returns the unix time of the time represented by a filename
// according to the time format specification. 
//
time_t NexradLdm::parseRealtimeLdmFilename( char *fileName, int& seqNum ){

  //
  // See if the string is long enough.
  //
  if ( strlen(fileName) < strlen( radarInputDir ) + 
                          strlen("YYYYMMDDhhmmssS") ) {
    return -1;
  }
  
  char *p = fileName + strlen( radarInputDir ) + 1; // Skip slash.
  
  date_time_t T;

  //
  // Initialize to zero, since this the value we want the field to
  // have if it is not listed in the format list.
  //
  T.year = 0; T.month = 0; T.day = 0;
  T.hour = 0; T.min = 0; T.sec = 0;

  //  
  // Loop through the dated directory format string, decoding the
  // filename as we go.
  //
  if (strlen(datedDirFormat)){
    for (unsigned id = 0; id < strlen(datedDirFormat); id++){
      int sc = getSignificanceCode (datedDirFormat[id]);
      switch(sc) {
      
      case 0 : // Year.
	if (1 != sscanf(p,"%4d", &T.year)){
	  POSTMSG( ERROR, "Unable to decode year from filename" );
	  return 0L;
	}
	p=p+4;
	break;
	
      case 1 : // Month.
	if (1 != sscanf(p,"%2d", &T.month)){
	  POSTMSG( ERROR, "Unable to decode month from filename" );
	  return 0L;
	}
	p=p+2;
	break;
	
      case 2 : // Day.
	if (1 != sscanf(p,"%2d", &T.day)){
	  POSTMSG( ERROR, "Unable to decode day from filename" );
	  return 0L;
	}
	p=p+2;
	break;
	
      case 3 : // Hour.
	if (1 != sscanf(p,"%2d", &T.hour)){
	  POSTMSG( ERROR, "Unable to decode hour from filename" );
	  return 0L;
	}
	p=p+2;
	break;
	
      default :
	p++; // Unrecognised char, must be part of the filename.
	break;
      }
    }
    p++; // Skip the slash on the dated directory.
  }

  //
  // Do the same for the file time format string.
  //
  for (unsigned it = 0; it < strlen(fileTimeFormat); it++){
    int sc = getSignificanceCode (fileTimeFormat[it]);
    switch(sc) {
      
    case 0 : // Year.
      if (1 != sscanf(p,"%4d", &T.year)){
	POSTMSG( ERROR, "Unable to decode year from filename" );
      return 0L;
      }
      p=p+4;
      break;
	
    case 1 : // Month.
      if (1 != sscanf(p,"%2d", &T.month)){
	POSTMSG( ERROR, "Unable to decode month from filename" );
	return 0L;
      }
      p=p+2;
      break;
	
    case 2 : // Day.
      if (1 != sscanf(p,"%2d", &T.day)){
	POSTMSG( ERROR, "Unable to decode day from filename" );
	return 0L;
      }
      p=p+2;
      break;
	
    case 3 : // Hour.
      if (1 != sscanf(p,"%2d", &T.hour)){
	POSTMSG( ERROR, "Unable to decode hour from filename" );
	return 0L;
      }
      p=p+2;
      break;
	
    case 4 : // Minute.
      if (1 != sscanf(p,"%2d", &T.min)){
	POSTMSG( ERROR, "Unable to decode minute from filename" );
	return 0L;
      }
      p=p+2;
      break;
	
    case 5 : // Second.
      if (1 != sscanf(p,"%2d", &T.sec)){
	POSTMSG( ERROR, "Unable to decode second from filename" );
	return 0L;
      }
      p=p+2;
      break;
	
    case 6 : // Sequence number.
      //
      // Should either have the letter E here (for End
      // of volume) or the integer sequence number.
      //
      if (*p == 'E'){
	seqNum = -1;
	p++;
      }
      else {
	if (1 != sscanf(p, "%d", &seqNum)){
	  POSTMSG( ERROR, "Unable to decode sequence from filename" ); 
	  return 0L;
	}
	p=p+1;
	if (realtimeSequenceNumber > 9) p=p+1;
	if (realtimeSequenceNumber > 99) p=p+1;
      }
      break;
      
    default :
      p++; // Unrecognised char, must be part of the filename.
      break;
    }
  }

  uconvert_to_utime( &T );

  return T.unix_time;

}


//
// Return an integer representing the significance
// of the character in thge TDRP time format definition.
// Code is relatively self explanitory.
//
int NexradLdm::getSignificanceCode (char FormatCode ){

  if (FormatCode == 'Y') return 0; // Year
  if (FormatCode == 'M') return 1; // Month.
  if (FormatCode == 'D') return 2; // Day.
  if (FormatCode == 'h') return 3; // Hour.
  if (FormatCode == 'm') return 4; // Minute.
  if (FormatCode == 's') return 5; // Second.
  if (FormatCode == 'S') return 6; // Sequence number.

  return -1; // Other character.

}

//
//
// Scan though a directory - return a valid filename, depending on
// the value of realtimeSequenceNumber.
//
int NexradLdm::getFile( char *directory, char *fileName )
{
   DIR *dirp = opendir(directory);
   if ( dirp == NULL ) {
      return -1;
   }

   struct dirent *dp;
   for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
    
      //
      // Those files which start with a dot or an underscore, they go
      // away.
      //
      if ( (dp->d_name[0] == '.') || (dp->d_name[0] == '_') ) {
         continue;
      }

      //
      // Get the full filename, and see if the file is
      // new enough.
      //
      char fullName[MAX_PATH_LEN];
      sprintf( fullName, "%s/%s", directory, dp->d_name );

      int    fileSeqNum   = -2;
      time_t fileTime     = parseRealtimeLdmFilename( fullName, fileSeqNum );
      time_t now          = time(NULL);
      long   age          = (now - fileTime);

      if ( age <= maxValidAgeMin*60 ) {

         //
         // OK, we have a filename.
         //
         if ( realtimeSequenceNumber != -1 ) {

            //
            // We need another file at the same time with either an
            // incremented sequence number, or the letter E.
            //
            if ( (fileTime == realtimeDatatime) && 
                 ( (fileSeqNum == realtimeSequenceNumber+1) || 
                   (fileSeqNum == -1) ) ) {
               closedir(dirp);
               sprintf(fileName,"%s",fullName);
               waitQtime(fullName);
               return 0;
            }
         }
         else {
            //
	    // We need a file with sequence number 0 and a time
	    // greater than the current time - ie. the first file in
	    // the next sequence. In this case, the routine will be called
	    // on both the current and the next directories.
	    //
	    int firstSeqNumInVol = 0;
	    if ( isBuild5 ) {
               firstSeqNumInVol = 1;
            }

	    if( (fileTime > realtimeDatatime) && 
                (fileSeqNum == firstSeqNumInVol) ) {

               bool goodToGo = true;

               //
	       // If we are in build 5, we need to check that this is
	       // in fact the start of a volume and not non-build 5 data.
	       //
	       if ( isBuild5 ) {
                  char firstFour[4];
	          FILE *fp = fopen(fullName,"r");
	          if (fp == NULL){
                     POSTMSG(ERROR, "Failed to open %s", fullName);
                     exit(-1);
                  }
               
                  if( fread( firstFour, sizeof(char), 4, fp ) != 4 ) {
                     POSTMSG(ERROR, "Failed to get first four bytes from %s", 
                             fullName);
	             exit(-1);
                  }
                  
                  fclose(fp);
                  
	          if ( strncmp( firstFour,"AR2V",4 ) ) {
                     goodToGo = false;
                  }
               }
            
	       if ( goodToGo ) {
                  closedir(dirp);
	          sprintf( fileName,"%s",fullName );
	          waitQtime(fullName);
	          return 0;
               }
            }
         }
      }
   }
  
   closedir(dirp);
   return -1;
}


///////////////////////////////////////////////////////
// CHANGE OUTPUT AND RETURN VALUES!!!
//////////////////////////////////////////////////////
//
// Put together the directory name to look for files in.
// If nextDir is set, we advance the time to look in the
// next directory, ie. for YYYYMMDD named directories,
// instead of 20031117 we look in 20031118.
//
int NexradLdm::getDir (char *dirName, bool nextDir) 
{
   //
   // Always returns 0. Result is passed out in dirName.
   //
   date_time_t T;

   T.unix_time = realtimeDatatime;
   uconvert_from_utime( &T );

   //
   // Advance this time to the next directory, if needs be.
   //
   if ( nextDir ) {

     //
     // See what the time increment needs to be.  We need
     // to figure out what the precision of the date directory
     // is.  In other words, what is the signficance of the
     // last digit in the directory name.
     //
     int maxSC = -1;
     for ( unsigned id = 0; id < strlen(datedDirFormat); id++ ) {
        
        int sc = getSignificanceCode (datedDirFormat[id]);
        if (sc > maxSC) {
           maxSC = sc;
        }
     }
     
     switch(maxSC) {

         case 0 :
            //
            // The year is the least significant time in the directory
            // naming convention.
            //
            T.year = T.year + 1;
            uconvert_to_utime( &T );
            break;

         case 1 :
            //
            // The month is the least significant time in the directory.
            // naming convention.
            T.month = T.month + 1;
            uconvert_to_utime( &T );
            break;

         case 2 :
            //
            // The day is the least significant time in the directory.
            // naming convention.
            T.unix_time = T.unix_time + 86400;
            uconvert_from_utime( &T );
            break;

         case 3 :
            //
            // The hour is the least significant time in the directory.
            // naming convention.
            //
            T.unix_time = T.unix_time + 3600;
            uconvert_from_utime( &T );
            break;

         default :
            //
            // There is no directory - all files are going into one
            // directory. Nothing to do.
            //
            break;
     }
   }

   sprintf( dirName, "%s", radarInputDir );

   if ( strlen(datedDirFormat) ) {

      sprintf(dirName,"%s%s",dirName,"/");
      
      for (unsigned id = 0; id < strlen(datedDirFormat); id++) {
         
         int sc = getSignificanceCode (datedDirFormat[id]);
         
         switch( sc ) {
      
             case 0 :
                sprintf( dirName,"%s%4d", dirName, T.year );
                break;
	
             case 1 :
                sprintf( dirName, "%s%02d", dirName, T.month );
	        break;
	
             case 2 : 
	        sprintf( dirName, "%s%02d", dirName, T.day );
	        break;
	
             case 3 :
	        sprintf( dirName, "%s%02d", dirName, T.hour );
                break;
	
             default :
                //
                // Unrecognised char, must be part of the filename.
                //
                sprintf(dirName,"%s%c", dirName, datedDirFormat[id]);
                break;
         }
      }
   }
   
   return 0;
}

//
// Set the sequence number to -1 (meaning that we are looking for
// the start of a new volume) and the start time to now minus
// the lookback time. This is called initially, and also called if
// a search times out.
//
void NexradLdm::resetFileSearchParameters()
{
  time_t now = time(NULL);
  realtimeDatatime = max( realtimeDatatime, now - maxValidAgeMin*60 );
  realtimeSequenceNumber = -1;
}


//
// This just waits until a file has been quiesecent for long enough.
//
void NexradLdm::waitQtime(char *fileName){

  long age;

  do {
    struct stat buf;
    
    if (stat(fileName, &buf)){
      perror("Error in stat.");
      exit(-1); // Highly unlikely.
    }

    age = time(NULL) - buf.st_mtime;
    
    long fileSize = buf.st_size;

    if ( (age < Qtime) || (fileSize < minFileSize) ) {
       sleep(1);
       PMU_auto_register("Waiting on quiesence.");
    }
  } while (age < Qtime);

  return;
}

void NexradLdm::readState() 
{
   //
   // Initialize
   //
   time_t now = time( NULL );

   realtimeDatatime = now - maxValidAgeMin*60;
   
   if( !usePrevState ) {
      return;
   }
   
   FILE *fp = fopen( stateFileName->c_str(), "r" );
   if( !fp ) {
      POSTMSG( WARNING, "Could not open file %s, using "
               "max valid age instead", stateFileName->c_str() );
      return;
   }
   
   char filename[FILE_NAME_LEN];
   if( !fgets( filename, FILE_NAME_LEN, fp ) ) {
      POSTMSG( WARNING, "Could not read file %s, using "
               "max valid age instead", stateFileName->c_str() );
      return;
   }
   
   realtimeDatatime = parseRealtimeLdmFilename( filename,
                                                realtimeSequenceNumber );

   fclose( fp );
}

int NexradLdm::writeState() 
{
   if( !realtimeMode || !usePrevState )
     return 0;

   FILE *fp = fopen( stateFileName->c_str(), "w" );
   if( !fp ) {
      POSTMSG( ERROR, "Could not open file %s", stateFileName->c_str() );
      return -1;
   }
   
   if( fputs( prevFilePath, fp ) == EOF ) {
      POSTMSG( ERROR, "Could not write current state to file %s",
               stateFileName->c_str() );
      return -1;
   }

   fclose( fp );
   
   return 0;
}
   

/////////////////////////////////////////


void NexradLdm::_byteSwap4(void *p){

  unsigned char *b = (unsigned char *)p;

  unsigned char b1 = *b;
  unsigned char b2 = *(b+1);
  unsigned char b3 = *(b+2);
  unsigned char b4 = *(b+3);


  *(b+3) = b1;
  *(b+2) = b2;
  *(b+1) = b3;
  *b = b4;

  return;

}
