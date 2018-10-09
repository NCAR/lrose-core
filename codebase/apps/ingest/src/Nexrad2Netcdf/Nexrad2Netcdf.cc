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
//////////////////////////////////////////////////////////////
//  Nexrad2Netcdf application
//
//  Refactored July 2007, Jason Craig
//
//  Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
//  July 2004
//
//  Adapted from nexrad2dsr application by Terri Betancourt 
//  RAP, NCAR, Boulder, CO, 80307, USA
//
//  $Id: Nexrad2Netcdf.cc,v 1.19 2017/12/28 18:17:28 jcraig Exp $
//
/////////////////////////////////////////////////////////////
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
#include <didss/DsInputPath.hh>

#include "Nexrad2Netcdf.hh"
#include "ReadNexrad.hh"

using namespace std;

//
// Revision History
//
// v1.0  Initial implementation
// v2.0  Uses latest data info for ldm, 
//       compression routines internal to 
//       this application - no need for system call
// v3.0  Refactored to remove complexity
//       and add ability to handle clutter data
//  3.1  Added Message 31 ability
//  3.2  Added PPI file support
//
const string Nexrad2Netcdf::version = "3.1";


Nexrad2Netcdf::Nexrad2Netcdf(Params *P, const char *programName)
{
  params = P;
  progName = programName;

  //
  // Register with procmap now that we have the instance name
  //
  PMU_auto_init( progName, params->instance, PROCMAP_REGISTER_INTERVAL );
  PMU_auto_register( "starting up application" );
  
  //
  // Some general stuff
  //
  msgLog.setApplication( progName );

  ucopyright( progName );

  //
  // Set debug level messaging
  //
  if ( params->debug ) {
    msgLog.enableMsg( DEBUG, true );
  } 

  //
  // Set info level messaging
  //
  if( params->info ) {
    msgLog.enableMsg( INFO, true );
  }

  readNexrad = new ReadNexrad(params);

}

Nexrad2Netcdf::~Nexrad2Netcdf()
{
  delete readNexrad;
}

int Nexrad2Netcdf::run(vector<string> inputFileList, time_t startTime, time_t endTime)
{
   DsInputPath *fileTrigger;

   //
   // Setup the DsInputPath based on our mode
   //
   if( params->mode == Params::REALTIME ) {
     POSTMSG( INFO, "Starting up in Realtime Mode");

     fileTrigger = new DsInputPath( progName,
				    params->debug,
				    params->radar_input_dir,
				    params->max_valid_age_min * 60,
				    PMU_auto_register,
				    true );
     
     string label = progName;
     label += "_";
     label += params->instance;
     
     fileTrigger->setSaveLatestReadInfo( label, true );
     
   }
   else if( params->mode == Params::FILE_LIST ) {
     
     if ( inputFileList.size() > 0 ) {
       POSTMSG( INFO, "Starting up in File List Mode, NFiles: %d", inputFileList.size());

       fileTrigger = new DsInputPath( progName,
				      params->debug,
				      inputFileList,
				      false );
     }
     else{
       POSTMSG( ERROR, "Must provide a file list on command line for file list mode" );
       return( -1 );
     }
   }
   else if( params->mode == Params::START_END ) {
     POSTMSG( INFO, "Starting up in Archive Mode");

     fileTrigger = new DsInputPath( progName,
				    params->debug,
				    params->radar_input_dir,
				    startTime,
				    endTime );
     
   }
   else {
     
     POSTMSG( ERROR, "Must use one of the available modes in "
	      "the parameter file" );
     return( -1 );
   }
   
   nexradInput.init( params->oneFilePerVolume, params->ppiFiles );

   bool processedFile = false;
   bool uncompress = true;
   time_t volumeTime = -1, curVolumeTime = -1;
   int sequenceNum = -1, prevSeqNum = -1;
   int versionNum;
   Nexrad2Netcdf::VolumeEnum_t fileType;
   char *filePath;
   vector<char *> fileVec;
   vector<int> fileSeqNumVec;
   vector<time_t> fileVolumeTimeVec;


   if( !params->oneFilePerVolume )
     uncompress = true;

   //
   // Loop over all the input files
   //
   while(filePath = fileTrigger->next()) {

     //
     // Volume files can just be processed as they come
     //
     if( params->oneFilePerVolume || params->mode == Params::FILE_LIST) {

       processFile(filePath, uncompress);
       if(params->oneFilePerVolume)
	 readNexrad->endOfData();
       processedFile = true;

     //
     // If this is not a volume file, we check the sequence numbers for order.
     //        
     } else {
       sequenceNum = getSequenceNum( filePath );
       versionNum = getVersionNum( filePath );
       fileType = getFileType( filePath );
       volumeTime = getVolumeTime( filePath );

       if(fileType == VOLUMEEND) {
	 //
	 // We found a volume end but it is not the current volume's
	 //
	 if(volumeTime > curVolumeTime) {
	   POSTMSG( WARNING, "Never received End of Volume for volume %d", curVolumeTime);

	   //char *newfilePath = replaceSequenceNum(filePath, prevSeqNum + 1, true, 0);
	   //if(processFile(newfilePath, uncompress) == -1) {
	     // Still can't find a file... give up.
	     //POSTMSG("Unable to find missing sequence number %d for volume %d. Rest of volume will be skipped.",
	   //     a, curVolumeTime);
	   //} else if(volumeTime < curVolumeTime) {

	   //}
	   prevSeqNum = -1;
	 }
	 sequenceNum = -1;
       }

       if(sequenceNum == -2)
         return( FAILURE );

       //
       // On Start up always start at the beginning of the volume
       //
       if(!processedFile) {
	 if(sequenceNum == 1) {
	   processFile(filePath, uncompress);
	   prevSeqNum = sequenceNum;
	   curVolumeTime = volumeTime;
	   processedFile = true;
	 } else {
	   if(prevSeqNum == -1) {
	     for(int a = 1; a < sequenceNum; a++) {
	       char *newfilePath = replaceSequenceNum(filePath, a, false, 0);
	       if(processFile(newfilePath, uncompress) == -1) {
		 POSTMSG( WARNING, "Unable to back up to start of volume.  Will wait for start of next volume.");
		 filePath = NULL;
		 prevSeqNum = -2;
		 a = sequenceNum;
	       }
	       delete[] newfilePath;
	     }
	     //
	     // Signals one of the files failed to process
	     if(filePath != NULL) {
	       processFile(filePath, uncompress);
	       prevSeqNum = sequenceNum;
	       curVolumeTime = volumeTime;
	       processedFile = true;
	     }
	   }
	 }

       } else {

	 int nextSeqNum = prevSeqNum + 1;
	 int nextSeqIndex;

	 // 
	 // Push every file into a holding vector
	 // Unless they have a -1 sequenceNum
	 //
	 if(sequenceNum != -1) {
	   char *cpyFilePath = new char[strlen(filePath)+1];
	   strcpy(cpyFilePath, filePath);
	   POSTMSG( INFO, "Pushing file %s, %d, %d", cpyFilePath, sequenceNum, volumeTime);
	   fileVec.push_back(cpyFilePath);
	   fileSeqNumVec.push_back(sequenceNum);
	   fileVolumeTimeVec.push_back(volumeTime);
	 }
	 if(sequenceNum == -1 || (params->ppiFiles && sequenceNum == 1))  {
	   // -1 means we found a End file.
	   // If we have any files in the vector from this volume time
	   // it means we missed at least one file.
	   // Check to see if we can find them.
	   int missedFiles = 0;
	   for(int a = 0; a < fileVec.size(); a++) {
	     if(fileVolumeTimeVec[a] == volumeTime &&
		fileSeqNumVec[a] > missedFiles) {
	       missedFiles = fileSeqNumVec[a];
	     }
	   }
	   if(params->ppiFiles && sequenceNum == 1) {
	     char *newfilePath = replaceSequenceNum(filePath, nextSeqNum, false, -1);
	     processFile(newfilePath, uncompress);
	   }

	   if(missedFiles != 0)
	   {
	     for(int a = nextSeqNum; a <= missedFiles; a++) {
	       if(a == 0) a++;
	       // See if the file is in the vector
	       bool found = false;
	       for(int b = 0; b < fileVec.size(); b++) {
		 if(fileVolumeTimeVec[b] == curVolumeTime &&
		    fileSeqNumVec[b] == a) {
		   processFile(fileVec[b], uncompress);  
		   delete[] fileVec[b];
		   fileVec.erase(fileVec.begin() + b);
		   fileSeqNumVec.erase(fileSeqNumVec.begin() + b);
		   fileVolumeTimeVec.erase(fileVolumeTimeVec.begin() + b);
		   found = true;
		   b = fileVec.size();
		 }
	       }
	       // Not in the vector, try to read it by name/number.
	       if(!found) {
		 char *newfilePath = replaceSequenceNum(filePath, a, false, 0);
		 if(processFile(newfilePath, uncompress) == -1) {
		   // Still can't find a file... give up.
		   POSTMSG("Unable to find missing sequence number %d for volume %d. Rest of volume will be skipped.",
			   a, curVolumeTime);
		   a = missedFiles;
		 }
		 delete[] newfilePath;
	       }
	     }
	   }
	 }

	 if(params->ppiFiles && sequenceNum == 1) {
	   prevSeqNum = -1;
	   nextSeqIndex = 1;
	 }

	 
	 //
	 // Find as many Sequence Numbers from our vector as are inorder
	 //
	 do {
	   nextSeqIndex = -1;

	   if(prevSeqNum == -1) {
	     // Last file was the last in that volume
	     // We must start with sequence number 1
	     for(int a = 0; a < fileVec.size(); a++) {
	       if(fileSeqNumVec[a] == 1)
		 nextSeqIndex = a;
	     }
	     
	   } else {
	     
	     for(int a = 0; a < fileVec.size(); a++) {
	       if(fileVolumeTimeVec[a] == curVolumeTime &&
		  fileSeqNumVec[a] == nextSeqNum)
		 nextSeqIndex = a;
	     }
	   }
	   
	   if(nextSeqIndex != -1) {
	     //
	     // Finally we can actually process the file
	     //
	     processFile(fileVec[nextSeqIndex], uncompress);       
	     prevSeqNum = fileSeqNumVec[nextSeqIndex];
	     curVolumeTime = fileVolumeTimeVec[nextSeqIndex];
	     
	     delete[] fileVec[nextSeqIndex];
	     fileVec.erase(fileVec.begin() + nextSeqIndex);
	     fileSeqNumVec.erase(fileSeqNumVec.begin() + nextSeqIndex);
	     fileVolumeTimeVec.erase(fileVolumeTimeVec.begin() + nextSeqIndex);
	     
	     //
	     // Starting a new volume
	     //
	     if(prevSeqNum == 1) {

	       //
	       // Throw out any files left in the vector
	       //
	       vector<char *>::iterator fileIterator = fileVec.begin();
	       vector<int>::iterator seqIterator = fileSeqNumVec.begin();
	       vector<time_t>::iterator volumeIterator = fileVolumeTimeVec.begin();
	       
	       while(fileIterator != fileVec.end() ) {
		 if(*volumeIterator < curVolumeTime) {
		   
		   POSTMSG( DEBUG, "Sequence number %d for volume %d left after moving on to volume %d", 
			    *seqIterator, *volumeIterator, curVolumeTime);

		   fileIterator = fileVec.erase( fileIterator );
		   seqIterator = fileSeqNumVec.erase( seqIterator );
		   volumeIterator = fileVolumeTimeVec.erase( volumeIterator );
		   
		 } else {
		   fileIterator++;
		   seqIterator++;
		   volumeIterator++;
		 }
	       }
	     } // if(prevSeqNum == 1)
	     
	   }  // if(nextSeqIndex != -1)
	   
	   if(sequenceNum == -1 && filePath != NULL) {
	     processFile(filePath, uncompress);
	     prevSeqNum = -1;
	     nextSeqIndex = 0;
	     sequenceNum = 0;
	   } else {	   
	     nextSeqNum = prevSeqNum + 1;
	   }

	 } while(nextSeqIndex != -1);

       } // if(!processedFile)
       
     } // if(params->oneFilePerVolume)
     

   } // while(filePath = fileTrigger->next())

   delete fileTrigger;

   if(processedFile == false) {
     POSTMSG( ERROR, "No files found within time specified" );
     return (-1);
   }

   readNexrad->endOfData();

   if( params->mode == Params::REALTIME ) {
     POSTMSG( ERROR, "Could not get next file" );
     return (-1);
   }

   POSTMSG( DEBUG, "End of file list" );

   return (0);
}

int Nexrad2Netcdf::processFile( char *filePath, bool uncompress)
{

  POSTMSG( INFO, "Opening file '%s'", filePath );
  
  
  //
  // Open the file and get the buffer of data, decompressing it if needed
  //
  if(nexradInput.readFile( filePath, uncompress ) != 0) {
    return (-1);
  }
  
  //
  // Send the buffer to read class
  //
  int msgCount = readNexrad->readBuffer((ui08 *)nexradInput.getBuffer(), 
				       nexradInput.getBytesRead(), nexradInput.cookie());
  if(msgCount < 0)
    return(-2);

  POSTMSG( DEBUG, "%d messages in file %s\n", msgCount, filePath );
  return(0);
}

int Nexrad2Netcdf::getSequenceNum( char *fileName )
{
   int seqNum = -2;

   char *s = strrchr( fileName, '/' );
   if(!s) {
      POSTMSG( ERROR, "No sequence number on file name" );
     return seqNum;
   }

   char *p = strchr( s, '_' );
   if( !p ) {
      POSTMSG( ERROR, "No sequence number on file name" );
      return seqNum;
   }

   if(params->ppiFiles) {
     p = strchr( p+1, '_' );
     if( !p ) {
       POSTMSG( ERROR, "No sequence number on file name" );
       return seqNum;
     }
     p = strchr( p+1, '_' );
     if( !p ) {
       POSTMSG( ERROR, "No sequence number on file name" );
       return seqNum;
     }

     if( 1 != sscanf( p, "_ELE%d", &seqNum ) ) {       
       POSTMSG( ERROR, "No sequence number found on file name" );
       return( -2 );
     }
     seqNum ++;

   } else {

     if( 1 != sscanf( p, "_%d", &seqNum ) ) {
       if( p[1] == 'E' ) {
         return( -1 );
       }
       
       POSTMSG( ERROR, "No sequence number found on file name" );
       return( -2 );
     }
   }

   return seqNum;

}

//
// Version numbers are only found in build 10 and later
// non fatal return if not found
// Build 10 Version numbers:
// 01: Legacy Message 1: Evansville (KVWX)
// 02: Message 31: Legacy resolution (Super-Res Generation disabled)
// 03: Message 31: Super-Resolution
// 04: Message 31: Recombined Super-Res 
int Nexrad2Netcdf::getVersionNum( char *fileName )
{
   int verNum = -2;

   char *p = strrchr( fileName, '_' );

   if( !p ) {
      return( -2 );
   }
   
   if( 1 != sscanf( p, "_V%d", &verNum ) ) {
      return( -2 );
   }

   return verNum;
}

//
// Get Volume file type
// E (end of volume), S (volume start), or I (intermediate) 
Nexrad2Netcdf::VolumeEnum_t Nexrad2Netcdf::getFileType( char *fileName )
{
   Nexrad2Netcdf::VolumeEnum_t verNum = VOLUMEUNKNOWN;

   char *s = strrchr( fileName, '/' );
   if(!s)
     return verNum;
   char *p = strchr( s, '_' );
   if(!p)
     return verNum;
   char *q = strchr( p+1, '_' );
   if(!q)
     return verNum;
   
   if(q[1] == 'I')
     verNum = VOLUMEINTERMEDIATE;
   if(q[1] == 'E')
     verNum = VOLUMEEND;
   if(q[1] == 'S')
     verNum = VOLUMESTART;

   return verNum;
}

struct conversion_failure { };

template <typename T>
T from_string (const std::string & s)
{
  T result;
  std::istringstream stream (s);
  if (stream >> result) return result;
  throw conversion_failure ();
}

time_t Nexrad2Netcdf::getVolumeTime( char *fileName )
{
  date_time_t timeStruct;
  size_t lastSlash, secondLastSlash;
  string cpyFileName(fileName);


  lastSlash = cpyFileName.find_last_of('/') +1;

  if(lastSlash == string::npos) {
    POSTMSG( ERROR, "Unable to determine volume time from file name" );
    return( -2 );
  }

  if(params->ppiFiles) {
    secondLastSlash = lastSlash;
    lastSlash = cpyFileName.find_first_of('_', lastSlash) + 1;
  } else {
    secondLastSlash = cpyFileName.find_last_of('/', lastSlash -2) + 1;
  }

  if(secondLastSlash == string::npos) {
    POSTMSG( ERROR, "Unable to determine volume time from file name" );
    return( -2 );
  }

  string yearS = cpyFileName.substr(secondLastSlash, 4);
  string monthS = cpyFileName.substr(secondLastSlash+4, 2);
  string dayS = cpyFileName.substr(secondLastSlash+6, 2);
  string hourS = cpyFileName.substr(lastSlash, 2);
  string minS = cpyFileName.substr(lastSlash+2, 2);
  string secS = cpyFileName.substr(lastSlash+4, 2);

  try {
    timeStruct.year = from_string<int>(yearS);
    timeStruct.month = from_string<int>(monthS);
    timeStruct.day = from_string<int>(dayS);
    timeStruct.hour = from_string<int>(hourS);
    timeStruct.min = from_string<int>(minS);
    timeStruct.sec = from_string<int>(secS);
  }
  catch( conversion_failure E ) {
    POSTMSG( ERROR, "Unable to determine volume time from file name" );
    return( -2 );
  }
  
  time_t rtime = uunix_time(&timeStruct);
  /* These check for unix time conversion error but are not neccessary 
  if(rtime < 1190000000) {
    POSTMSG( ERROR, "Small unix time %d detected, %d, %d, %d, %d, %d, %d, file %s, chars: '%s' '%s' '%s' '%s' '%s' '%s'",
	     rtime, timeStruct.year, timeStruct.month, timeStruct.day, timeStruct.hour, timeStruct.min, timeStruct.sec, fileName,
	     yearS.c_str(), monthS.c_str(), dayS.c_str(), hourS.c_str(), minS.c_str(), secS.c_str());
  }
  if(rtime > 1200000000) {
    POSTMSG( ERROR, "Large unix time %d detected, %d, %d, %d, %d, %d, %d, file %s, chars: '%s' '%s' '%s' '%s' '%s' '%s'",
	     rtime, timeStruct.year, timeStruct.month, timeStruct.day, timeStruct.hour, timeStruct.min, timeStruct.sec, fileName,
	     yearS.c_str(), monthS.c_str(), dayS.c_str(), hourS.c_str(), minS.c_str(), secS.c_str());
  }
  */
  return rtime;
}

char *Nexrad2Netcdf::replaceSequenceNum( char *fileName, int newNumber, bool setEnd, int sequencingOffset )
{
   char *newfileName = new char[strlen(fileName)+3];
   strcpy(newfileName, fileName);

   char *p = strrchr( newfileName, '_' );

   if( !p ) {
      POSTMSG( ERROR, "No sequence number on file name" );
      return NULL;
   }

   if( p[1] == 'V')
     p = p -5;

   if(params->ppiFiles) {
     newNumber --;
     if(p[1] == 'E' && p[2] == 'L' && p[3] == 'E')
       p = p +3;
   }

   if( p[6] == 'V') {
     char numString[6];
     if(newNumber == 1) {
       sprintf( numString, "_%02d_S", newNumber);
     } else {
       if(setEnd)
	 sprintf( numString, "_%02d_E", newNumber);
       else
	 sprintf( numString, "_%02d_I", newNumber);
     }
     p[0] = numString[0];
     p[1] = numString[1];
     p[2] = numString[2];
     p[3] = numString[3];
     p[4] = numString[4];
     
   } else if( p[1] == 'E') {

     char numString[4];
     sprintf( numString, "_%02d", newNumber);

     for(int a = strlen(p); a >= 0; a--)
       p[a+1] = p[a];

     p[0] = numString[0];
     p[1] = numString[1];
     p[2] = numString[2];

   } else {
     if(setEnd) {

     for(int a = 0; a < strlen(p)-1; a++)
       p[a] = p[a+1];

     p[0] = '_';
     p[1] = 'E';

     } else {
       p += 1;
       char numString[4];
       sprintf( numString, "%02d", newNumber);
       p[0] = numString[0];
       p[1] = numString[1];
     }
   }
   if(sequencingOffset != 0) {
     char *p = strrchr( newfileName, '.' );
     p[0] = char(0);
     char *p2 = strrchr( newfileName, '.' );

     int newSequencing = atoi(p2+1) + sequencingOffset;
     char numString[5];
     sprintf( numString, "%03d", newSequencing);
     p2[1] = numString[0];
     p2[2] = numString[1];
     p2[3] = numString[2];
     p[0] = '.';
   }

   return newfileName;
}
