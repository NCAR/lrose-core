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
//  Nexrad sub-class for reading from an LDM radar stream
//
//  Responsible for reading the input data from a LDM file
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: NexradLdm.cc,v 1.42 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>

#include <bzlib.h>

#include "Driver.hh"
#include "NexradLdm.hh"
using namespace std;

NexradLdm::NexradLdm()
          :NexradInput()
{
   dsTrigger    = NULL;
   fileTrigger  = NULL;
   ldmFile      = NULL;
   fileIsOpen   = false;
   compression  = UNCOMPRESSED;
   bytesLeft    = 0;
   byteOffset   = 0;
   noVolumeTitleYet = true;
}

NexradLdm::~NexradLdm()
{
   delete fileTrigger;
   delete dsTrigger;

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
   // bool libraryDebug = true; -- Not used now.

   radarInputDir = strdup( params.radar_input_dir );
   datedDirFormat = strdup( params.dated_dir_format );
   fileTimeFormat = strdup( params.file_time_format );
   instance = strdup( params.instance );
   maxValidAgeMin = params.max_valid_age_min;
   maxElapsedSearchTimeMin = params.max_elapsed_search_time;
   Qtime = params.file_quiescence_sec;
   minFileSize = params.min_file_size;
   isBuild5 = params.build5;
   oneFilePerVolume = params.oneFilePerVolume;
   decompress_mechanism = params.decompress_mechanism;

   if ( params.mode == Params::REALTIME ) {
      realtimeMode = true;
      POSTMSG( DEBUG, "Initializing realtime input from '%s'.",
                       params.radar_input_dir );
      resetFileSearchParameters();

      /* --------------- Don't do this now.
      fileTrigger = new DsInputPath( PROGRAM_NAME, libraryDebug,
                                     params.radar_input_dir,
                                     params.max_valid_age_min*60,
                                     PMU_auto_register,
                                     params.ldata_info_avail );
      fileTrigger->setFileQuiescence( params.file_quiescence_sec );
      fileTrigger->setDirScanSleep( params.input_probe_sec );
      */
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

/*----don't use the DsInputPath for archive mode processing 
      until alphabetizing contstraints get sorted out...

         fileTrigger = new DsInputPath( PROGRAM_NAME, libraryDebug,
                                        inputFileList );
      }
      else {
         DateTime startTime = driver->getStartTime();
         DateTime endTime   = driver->getEndTime();
         POSTMSG( DEBUG, "Initializing archive input between %s and %s",
                         startTime.dtime(), endTime.dtime() );
         fileTrigger = new DsInputPath( PROGRAM_NAME, libraryDebug,
                                        params.radar_input_dir,
                                        startTime.utime(), endTime.utime() );
      }
-------------------------------------------------------------*/
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
   tmpPath = params.tmp_dir;
   tmpPath += "/nexrad2dsr.uncompressed.data.";
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
              // Continue onto the next record and/or file
	      //
              // The following line commented out on suggestion
	      // from Jaimi Yee March 22 2005 - Niles Oien.
	      //
	      readLogicalRecord();
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

   //
   // Swap bytes to get message_len correct.
   //
   unsigned char *b = (unsigned char *) msgHdr;
   unsigned char tmp = *b;
   *b = *(b+1);
   *(b+1) = tmp;

   // int msgLen = ( msgHdr->message_len );

   // POSTMSG( DEBUG, "Message type, length : %d, %d", msgType, msgLen );

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
         POSTMSG( ERROR, "Unable to read from input file" );
         closeLdmFile();
         physicalBytes = 0;
         return( Status::BAD_INPUT_STREAM );
      }

      if ( 
	  (strncmp( ((RIDDS_vol_title*)ldmBuffer)->filename, "ARCHIVE2", 8 )) &&
	  (strncmp( ((RIDDS_vol_title*)ldmBuffer)->filename, "AR2V", 4 ))
	  ) {
         //
         // Nope, not a volume title, rewind to the beginning of the file
         // so we can read in the entire nexrad record
         //
         POSTMSG( DEBUG, "Not a volume title, rewinding to beginning of file" );
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
	 if ((isBuild5) && (!(oneFilePerVolume))){
	   //
	   POSTMSG( DEBUG, "Skipping build 5 metadata file" );
	   closeLdmFile();
	   physicalBytes = 0;
	   return( Status::END_OF_FILE );
	 }

         // Make note of the fact that we hit a volume title boundary
         // This information will be used as an additional check for
         // startOfVolume since the nexrad data stream seems to be
         // a bit remiss in setting the radial_status flags.
         //
         volumeTitleSeen = true;

 
	 char name[9];
	 char ext[5];
	 name[8] = ext[4] = '\0';
	 
	 strncpy( name, ((RIDDS_vol_title *)ldmBuffer)->filename, 8 );
	 strncpy( ext, ((RIDDS_vol_title *)ldmBuffer)->extension, 4 );
	 
	 if ( DEBUG_ENABLED ) {
	   POSTMSG( DEBUG, "Volume title: %s.%s", name, ext );
	   
	   if (isBuild5){
	     POSTMSG( DEBUG, "Assuming build 5 data." );
	   } else {
	     POSTMSG( DEBUG, "Assuming data are not build 5." );
	   }
	 }
	 /////////////////////////////////////////////////////
	 //
	 // If this is build 5 data, we have to skip the
	 // metadata that immediately follows the volume
	 // header.
	 //
	 if (isBuild5){
	   ui32 sizeRecordHeader;
	   int numRead;
	   numRead = fread( &sizeRecordHeader, sizeof(ui32),
			    1, ldmFile );
	   
	   if (numRead != 1){
	     perror("Failed to get metadata size.");
	     return( Status::BAD_INPUT_STREAM );
	   }
	   
	   _byteSwap4(&sizeRecordHeader);
	   
	   if (0 != fseek(ldmFile, sizeRecordHeader, SEEK_CUR)){
	     perror("Failed to skip metadata.");
	     return( Status::BAD_INPUT_STREAM );
	   }
	   //
	   closeLdmFile();
	   physicalBytes = 0;
	   return( Status::END_OF_FILE );
	 }
	 //
	 // End of skipping the metadata.
	 //
	 //////////////////////////////////////////////////// 
      }
   }

   //
   // If we have not seen a volume title yet, close the file
   // and return - we cannot proceed until we know if these
   // are build 5 data or not, and that is determined by the volume
   // header.
   //
   /*------------------------ Not so now that we set this in TDRP
   if (noVolumeTitleYet){
     POSTMSG( DEBUG, "No volume header encountered in input, skipping file." );
     closeLdmFile();
     physicalBytes = 0;
     return( Status::END_OF_FILE );
   }
   ---------------------------------------*/
   //
   // Get a block of 100 NEXRAD records from the file.
   // Again, this depends if the data are build 5 or not, and if
   // they are delivered as one file per volume.
   //

   if ((isBuild5) && (oneFilePerVolume)){
     si32 sizeRecordHeader;
     int numRead;
     numRead = fread( &sizeRecordHeader, sizeof(si32),
		      1, ldmFile );
   
     if (numRead != 1){
       perror("Failed to get size for 100 beam data.");
       return( Status::BAD_INPUT_STREAM );
     }
   
     _byteSwap4(&sizeRecordHeader);

     if (sizeRecordHeader < 1 ){
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

     if (inCompressedData == NULL){
       POSTMSG(ERROR,"Malloc failed.");
       exit(-1);
     }

     numRead = fread( inCompressedData, sizeof(ui08),
		      sizeRecordHeader, ldmFile );

     if (numRead != sizeRecordHeader){
       perror("Failed to get 100 beam data.");
       return( Status::BAD_INPUT_STREAM );
     }
     //
     // Decompress these data.
     //
     unsigned int numOut=NEX_BUFFER_SIZE;
     int r = BZ2_bzBuffToBuffDecompress ( (char *)ldmBuffer,
					&numOut,
					inCompressedData,
					sizeRecordHeader,
					0,0);

     if (r != BZ_OK){
       fprintf(stderr,"BZ failure.\n");
       exit(-1);
     }

     free (inCompressedData);
     bytesRead = numOut;

   } else {
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
   ldmFile = NULL;
}

Status::info_t
NexradLdm::openNextFile()
{
   char*  filePath;

   POSTMSG( DEBUG, "Fetching next input data file with sequence number %d",
	   realtimeSequenceNumber );

   //
   // Get the next file path, skipping over any directories
   //
   // if ( fileTrigger ) {
   //
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
     //
     if (fileTrigger) {
       //
       // Archive mode - can just trust the order of the files.
       //
       filePath = fileTrigger->next();
       //
       if ( !filePath ) {
	 POSTMSG( DEBUG, "Completed processing all input data files." );
	 return( Status::END_OF_DATA );
       }
     }
     else {
       if (NULL==dsTrigger){
	 POSTMSG( DEBUG, "dsTrigger unititalized - exiting.");
	 exit(-1);
       }
       TriggerInfo triggerInfo;
       int status = dsTrigger->next( triggerInfo );
       if ( status == -1 ) {
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
   if ( compression == UNCOMPRESSED ) decompress = false;
   if (
       (realtimeSequenceNumber == 1) && 
       (isBuild5) &&
       (!(oneFilePerVolume))
       ){
      decompress = false;
   }

   if ( decompress ) {
      char   ctype[6];
      char   command[2048];

      //
      // Remove the temporary file from before
      //
      unlink( tmpPath.c_str() );

      if ( compression == BZIP2 ) {
         strcpy( ctype, "BZIP2" );
      }
      else {
         strcpy( ctype, "ZLIB" );
      }

      //
      // Remove any earlier temporary files.
      //
      unlink( tmpPath.c_str() );

      //
      // Wait until it has at least 24 bytes of data in it.
      // experience has shown that this is necessary - otherwise
      // it may appear as having 0 bytes in it for a while.
      //
      int fileSize = 0;
      struct stat Buf;
      
      do {
	umsleep(100);
	if (stat(filePath, &Buf)){
	  fileSize = 0;
	} else {
	  fileSize = Buf.st_size;
	}
      } while (fileSize < 24);


      //
      // Now uncompress into the temporary file.
      //
      POSTMSG( DEBUG, "Uncompressing input file with command :" );

      switch( decompress_mechanism ){

      case Params::DECOMPRESS_NEXRADII_BZ :

	sprintf( command, "nexradII_bz -C %s %s < %s",
		 ctype, tmpPath.c_str(), filePath );
	break;

      case Params::DECOMPRESS_NEXRAD_BZIP_DECOMPRESS :

	sprintf( command, "NexradBzipDecompress %s %s %s",
		 filePath, tmpPath.c_str(), instance );
	break;

      default :
	POSTMSG (ERROR, "Unrecognised decompress option %d",
		 decompress_mechanism );
	exit(-1);
	break;

      }

      POSTMSG( DEBUG, command );

      system( command );

      //
      // Wait until the file seems to hold some data (or until
      // we time out).
      //
      int fileCheckCount = 0;
      int go = 1;
      do {

	PMU_auto_register("Waiting for uncompress command to work.");
	umsleep(100); // Sleep 100 milliseconds, ie. a tenth of a second.
	struct stat buf;
	
	if (0 == stat(tmpPath.c_str(), &buf)){
	  if (buf.st_size > 24){
	    go = 0; // We're done.
	  }
	}
	fileCheckCount++;
	// fprintf(stderr,"Check %d done.\n", fileCheckCount);
      } while((go) && (fileCheckCount < 50));


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
      //
      // Do a 'stat' on the file, make sure that it's there and is
      // of non-zero size. If that's not the case then there's a
      // compression issue.
      //

      struct stat buf;

      if (stat(filePath, &buf)){
	POSTMSG(ERROR, "Uncompressed file '%s' does not exist.", filePath);
	POSTMSG(ERROR, "Check compression settings, temporary file directory.");
	exit(-1);
      }

      if (buf.st_size < 24){
	POSTMSG(ERROR, "Uncompressed file '%s' only holds %d bytes.", filePath, (int)buf.st_size);
	POSTMSG(ERROR, "Check compression settings, temporary file directory.");
	exit(-1);
      }

      POSTMSG( DEBUG, "Uncompressed file '%s' holds %d bytes.", filePath, (int)buf.st_size);

   }

   //
   // Open the file readOnly
   //
   ldmFile = fopen( filePath, "r" );
   if ( ldmFile == NULL ) {
      POSTMSG( ERROR, "Unable to open file '%s'", filePath );
      perror( filePath );
      return( Status::BAD_INPUT_STREAM );
   }

   //
   // Successfully opened the file
   //
   POSTMSG( DEBUG, "File '%s' opened successfully", filePath );
   fileIsOpen = true;
   return( Status::ALL_OK );
}




//
// Returns either the next realtime file or NULL if things did
// not go well. If NULL is returned, the search parameters have been
// reset (ie. we start looking again with a clean slate, not trying to follow
// a sequence).
//
char* NexradLdm::getNextLdmRealtimeFile(){
  //
  time_t searchStartTime = time(NULL);
  //
  if (realtimeSequenceNumber < 0){
    //
    // OK - need something with sequence number 0 and
    // a data time greater than the last data time. In this
    // case we need to look at the next dated data dir, too,
    // because we may have rolled over into the next day.
    //
    int loop = 1;
    int loopCount = 0;
    char file[MAX_PATH_LEN];
    do {
      char dir[MAX_PATH_LEN];
      getDir(dir, false);

      if (getFile(dir, file)){
	//
	// Check the next directory.
	//
	getDir(dir, true);
	if (!(getFile(dir, file))){
	  loop = 0;
	}
      } else {
	loop = 0;
      }
      sleep(1); // See comment below about why I sleep for a second.
      loopCount++;
      if (loopCount % 5 == 0){
	PMU_auto_register("Looking for start of new volume");
      }
      int elapsedSearchTime = time(NULL) - searchStartTime;
      if (elapsedSearchTime > 60*maxElapsedSearchTimeMin){
	POSTMSG( DEBUG, "NexradLdm::getNextLdmRealtimeFile() : Elapsed search time for file exceeded. Check data latency.");      
	resetFileSearchParameters();
	return NULL;
      }
    } while (loop);

    sprintf(fileNameBuffer, "%s", file);

  } 
  else {
    //
    // Know pretty much what we are looking for - a file with
    // exactly the same data time and an incremented sequence
    // number (Or the letter 'E' indicating the end of the sequence).
    // In this case there is no need to look at the next dated dir.
    //
    int loop = 1;
    int loopCount = 0;
    char file[MAX_PATH_LEN];
    do {
      char dir[MAX_PATH_LEN];
      getDir(dir, false);
      if (!(getFile(dir, file))){
	loop = 0;
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
      int elapsedSearchTime = time(NULL) - searchStartTime;
      if (elapsedSearchTime > 60*maxElapsedSearchTimeMin){
	resetFileSearchParameters();
	return NULL;
      }
    } while (loop);
      
    sprintf(fileNameBuffer, "%s", file);

  }

  realtimeDatatime = parseRealtimeLdmFilename(fileNameBuffer,
					      &realtimeSequenceNumber);

  POSTMSG( DEBUG, "Found file : %s", fileNameBuffer );
  POSTMSG( DEBUG, "The time has been set to %s",
	   utimstr( realtimeDatatime ));
  POSTMSG( DEBUG, "The sequence number is %d",
	   realtimeSequenceNumber );

  return fileNameBuffer;

}

// 
// This returns the unix time of the time represented by a filename
// according to the time format specification. It also returns
// the sequence number. Returns 0L as the sequence number if
// things did not go well.
//
time_t NexradLdm::parseRealtimeLdmFilename(char *fileName,
					   int *seqNum){

  //
  // See if the string is long enough.
  //
  if (strlen(fileName) < strlen( radarInputDir ) +
      strlen("YYYYMMDDhhmmssS")){
    return -1;
  }
  
  char *p = fileName + strlen( radarInputDir ) + 1; // Skip slash.
  
  date_time_t T;
  //
  // Initialize to zero, since this the value we want the field to
  // have if it is not listed in the format list.
  //
  T.year = 0; T.month = 0; T.day = 0;
  T.hour = 0; T.min = 0;   T.sec = 0;
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
	*seqNum = -1;
	p++;
      }
      else {
	if (1 != sscanf(p,"%d", seqNum)){
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
int NexradLdm::getFile(char *directory, char *fileName){


  DIR *dirp;
  dirp = opendir(directory);

  if (dirp == NULL){
    return -1;
  }

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
    //
    // Those files which start with a dot or an underscore, they go
    // away.
    //
    if (
	(dp->d_name[0] == '.') ||
	(dp->d_name[0] == '_')
	){
      continue;
    }
    //
    // Get the full filename, and see if the file is
    // new enough.
    //
    char fullName[MAX_PATH_LEN];
    sprintf(fullName, "%s/%s", directory, dp->d_name);
   
    /*
    POSTMSG(DEBUG, "getFile(): NAME : %s", fullName);
    */
    
    /*----------------------------
      I do not use stat to get the file time, it takes
      too long at the end of the day when the directory is full
      of files and we have to stat them all. I parse the data
      time from the filename instead. Niles.
    struct stat buf;

   if (stat(fullName, &buf)){
     perror("Error in stat.");
     return -1;
   }
    -------------------------------*/

    time_t fileDataTime;
    int dummy;

    fileDataTime = parseRealtimeLdmFilename(fullName, 
					    &dummy);

 
   time_t now = time(NULL);
   //   long age = (now - buf.st_ctime);
   long age = (now - fileDataTime);

   /*
   POSTMSG(DEBUG, "TIME : %s AGE : %ld SEQ : %d", 
   	   utimstr(fileDataTime), age, realtimeSequenceNumber);
   */

   if (age <= maxValidAgeMin*60){
     //
     // OK, we have a filename.
     //
     if (realtimeSequenceNumber != -1){
       //
       // We need another file at the same time with either an
       // incremented sequence number, or the letter E.
       //
       time_t fileTime;
       int fileSeqNum;
       fileTime = parseRealtimeLdmFilename(fullName, 
					   &fileSeqNum);
       if (
	   (fileTime == realtimeDatatime) &&
	   (
	    (fileSeqNum == realtimeSequenceNumber+1) ||
	    (fileSeqNum == -1)
	    )
	   ){
	 closedir(dirp);
	 sprintf(fileName,"%s",fullName);
	 waitQtime(fullName);
	 return 0;
       }
     } else {
	 //
	 // We need a file with sequence number 0 and a time
	 // greater than the current time - ie. the first file in
	 // the next sequence. In this case, the routine will be called
	 // on both the current and the next directories.
	 //
	 time_t fileTime;
	 int fileSeqNum;
	 fileTime = parseRealtimeLdmFilename(fullName, 
					     &fileSeqNum);
	 int firstSeqNumInVol = 0;
	 if (isBuild5) firstSeqNumInVol = 1;

	 if (
	     (fileTime > realtimeDatatime) &&
	     (fileSeqNum == firstSeqNumInVol)
	     ){

	   bool goodToGo = true;
	   //
	   // If we are in build 5, we need to check that this is
	   // in fact the start of a volume and not non-build 5 data.
	   //
	   if (isBuild5){
	     char firstFour[4];
	     FILE *fp = fopen(fullName,"r");
	     if (fp == NULL){
	       POSTMSG(ERROR, "Failed to open %s", fullName);
	       exit(-1);
	     }
	     int numRead = fread(firstFour, sizeof(char),4,fp);
	     if (numRead != 4){
	       POSTMSG(ERROR, "Failed to get first four bytes from %s", fullName);
	       exit(-1);
	     }
	     fclose(fp);
	     if (strncmp(firstFour,"AR2V",4)){
	       goodToGo = false;
	     }
	   }

	   if (goodToGo){
	     closedir(dirp);
	     sprintf(fileName,"%s",fullName);
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


//
// Put together the directory name to look for files in.
// If nextDir is set, we advance the time to look in the
// next directory, ie. for YYYYMMDD named directories,
// instead of 20031117 we look in 20031118.
int NexradLdm::getDir (char *dirName, bool nextDir){
  //
  // Always returns 0. Result is passed out in dirName.
  //
  date_time_t T;

  T.unix_time = realtimeDatatime;
  uconvert_from_utime( &T );
  //
  // Advance this time to the next directory, if needs be.
  //
  if (nextDir){
    //
    // See what the time increment needs to be - depends
    // on the maximum time increment in the directory name
    // format.
    //
    int maxSC = -1;
    for (unsigned id = 0; id < strlen(datedDirFormat); id++){
      int sc = getSignificanceCode (datedDirFormat[id]);
      if (sc > maxSC){
	maxSC = sc;
      }
    }
    switch(maxSC){
    case 0 :
      // The year is the least significant time in the directory
      // naming convention.
      T.year = T.year + 1;
      uconvert_to_utime( &T );
      break;

    case 1 :
      // The month is the least significant time in the directory.
      // naming convention.
      T.month = T.month + 1;
      uconvert_to_utime( &T );
      break;

    case 2 :
      // The day is the least significant time in the directory.
      // naming convention.
      T.unix_time = T.unix_time + 86400;
      uconvert_from_utime( &T );
      break;

    case 3 :
      // The hour is the least significant time in the directory.
      // naming convention.
      T.unix_time = T.unix_time + 3600;
      uconvert_from_utime( &T );
      break;

    default :
      // There is no directory - all files are going into one
      // directory. Nothing to do.
      break;
    }
  } // End of if we have to advance to the next directory.

  sprintf(dirName,"%s",radarInputDir);


  if (strlen(datedDirFormat)){
    sprintf(dirName,"%s%s",dirName,"/");
    //
    for (unsigned id = 0; id < strlen(datedDirFormat); id++){
      int sc = getSignificanceCode (datedDirFormat[id]);
      switch(sc) {
      
      case 0 : // Year.
	sprintf(dirName,"%s%4d",dirName,T.year);
	break;
	
      case 1 : // Month.
	sprintf(dirName,"%s%02d",dirName,T.month);
	break;
	
      case 2 : // Day.
	sprintf(dirName,"%s%02d",dirName,T.day);
	break;
	
      case 3 : // Hour.
	sprintf(dirName,"%s%02d",dirName,T.hour);
	break;
	
      default :
        // Unrecognised char, must be part of the filename.
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
void NexradLdm::resetFileSearchParameters(){

  time_t now = time(NULL);

  POSTMSG(DEBUG, "The input search parameters were reset at %s",
	  utimstr(now));

  realtimeDatatime = now - maxValidAgeMin*60;
  realtimeSequenceNumber = -1;

  return;

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

    if (
	(age < Qtime) ||
	(fileSize < minFileSize)
	){
      sleep(1);
      PMU_auto_register("Waiting on quiesence.");
    }

  } while (age < Qtime);

  return;
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
