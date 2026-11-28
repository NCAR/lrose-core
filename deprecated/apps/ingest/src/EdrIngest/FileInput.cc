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
/////////////////////////////////////////////////////////////////////////////////
//
//  FileInput is sub-class of EdrInput for reading from file
//  Sue Dettling, RAP, NCAR, Boulder, CO, 80307, USA
//  November 2004
//
//  
////////////////////////////////////////////////////////////////////////////////

#include "FileInput.hh"

using namespace std;

FileInput::FileInput()
          :EdrInput()
{

}

FileInput::~FileInput()
{

  delete fileTrigger;
  
  if (fileIsOpen)
    fclose(edrFile);
}

int FileInput::init( Params& parameters,  const vector<string> &fileList , string &prog)
{
  params = parameters;

  progName = prog;

  fileIsOpen = false;

  msgsReadPerFile = 0;

  
  if ( params.mode == Params::REALTIME ) 
    {
      if (params.debug)
	cerr << "FileInput::init: Initializing realtime input from " 
	     << params.input_dir << endl;
      
      fileTrigger = new DsInputPath( progName, params.debug,
				     params.input_dir,
				     params.max_valid_age_min*60,
				     PMU_auto_register,
				     params.ldata_info_avail,
				     false );
      //
      // Set wait for file to be written to disk before being served out.
      //
      fileTrigger->setFileQuiescence( params.file_quiescence_sec );
      
      //
      // Set time to wait before looking for new files.
      // 
      fileTrigger->setDirScanSleep( params.check_input_sec );
      
    }

   if ( params.mode == Params::FILELIST ) 
    {    
      //
      // Archive mode.
      //
      if ( fileList.size() ) 
	{
	  if (params.debug)
	    cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;
	  
	  fileTrigger = new DsInputPath( progName,params.debug ,
					 fileList );
	}
    }

   if ( params.mode == Params::TIME_INTERVAL ) 
    { 
      //
      // Set up the time interval
      //
      DateTime start( params.start_time);
      
      DateTime end( params.end_time);
      
      time_t time_begin = start.utime();
      
      time_t time_end = end.utime();

       if (params.debug)
	 {
	    cerr << "FileInput::init: Initializing file input for time interval [" 
		 << time_begin << " , " << time_end << "]." << endl;

            cerr << "Output saved in " << params.output_url << endl;

	 }

      fileTrigger = new DsInputPath( progName, params.debug,
				     params.input_dir,
				     time_begin,
				     time_end);
    }


   return( 0 );
}

//////////////////////////////////////////////////////////////////
//
// FileInput::readFile: Depending on the file format, read entire 
//                      message into buffer (for ASCII data) or just
//                      the filename (for BUFR) data.
// 
EdrReport::status_t FileInput::readFile( ui08* &buffer, DateTime &msgTime)
{
  EdrReport::status_t ret;

  if (params.message_format == Params::ASCII)
    ret = readEdrMsg(buffer, msgTime);

  else 
    //
    // params.message_format == Params::BUFR
    //
    ret = getEdrFile(buffer, msgTime);

  return (ret);

}
  


//////////////////////////////////////////////////////////////
//
//  FileInput::getEdrFile: Get the next filename and record
//                         the filetime from the filepath.
//                         This is used for UAL BUFR messages
//                         since the 3rd party mel_bufr lib
//                         does opening , closing and parsing of file.
//                         
EdrReport::status_t FileInput::getEdrFile(ui08* &filename,  DateTime &msgTime)
{
  char * newfile = fileTrigger->next();
      
  if (newfile == NULL)
    return EdrReport::END_OF_DATA;
 
  getFileTime(newfile);
  
  msgTime.set(year, month, day, hour, min, sec);

  sprintf((char*)filename, "%s", newfile);

  return EdrReport::ALL_OK;
}


EdrReport::status_t FileInput::readEdrMsg( ui08* &buffer , DateTime &msgTime)
{

  if (!fileIsOpen)
    {
      char* newFile = fileTrigger->next();
      
      if (newFile == NULL)
	return EdrReport::END_OF_DATA;

      //
      // record year, month, day, hour, minute, second of message time
      //
      getFileTime(newFile);
      
      edrFile = fopen( newFile, "r" );

      if (!edrFile)
	{
	  cerr << "FileInput::readEdrMsg: ERROR: Unable to open file " << newFile << endl;

	  return( EdrReport::BAD_INPUT_STREAM );
	}

      if (params.debug)
	{
	  cerr << "\n\nFileInput::readEdrMsg: Reading  " << newFile << endl; 
	}

      fileIsOpen = true;

      sprintf(currFileName, newFile);
    }
 
  //
  // Set message time. (There may be more than one message per file
  // so we set outside of above if statement for 2nd and following
  // messages.)
  //
  msgTime.set(year, month, day, hour, min, sec);
 
  //
  // Find the beginning of the next message and its length and copy
  // it to the input buffer.
  // 
  int msgLen = 0;

  int msgStart = -1;

  int msgEnd = -1;
  
  bool recordingLen = false;
  
  while( !(feof(edrFile)) )
    {
      int c = fgetc(edrFile);
      
      //
      // If character is soh (start of header), mark the start
      //
      if (c == 1)
	{
	  msgStart = ftell(edrFile);
	  
	  recordingLen = true;
	}
      
      //
      // If character is etx( end of text) , mark the end.
      //
      if (c == 3) 
	msgEnd = ftell(edrFile);
      
      if ( recordingLen)
	msgLen++;

      //
      // If both the start and end offsets have been set, copy
      // the message to the buffer.
      //
      if (msgStart > -1 && msgEnd > -1 && (msgStart <= msgEnd))
	{
	  //
	  // Set the file position indicator to the beginning of the message.
	  //
	  int ret = fseek(edrFile, msgStart - 1, SEEK_SET);
	  
	  //
	  // Read the message characters and store them in input buffer.
	  //
	  fread((void *)buffer, sizeof(char), msgLen, edrFile);
	  
	  //
	  // Terminate the message string
	  //
	  buffer[msgLen] = '\0';
	  
	  //
	  // Set the file position indicator to the end of the message.
	  //
	  ret = fseek(edrFile, msgEnd, SEEK_SET);
	  

	  msgsReadPerFile++;

	  if (params.debug == Params::DEBUG_VERBOSE)
	    {
	      cerr << "FileInput::readEdrMsg: " << msgsReadPerFile << " messages read from " 
		   << currFileName << endl; 
	    }

	  return EdrReport::ALL_OK;

	}
    }
  
  if (feof(edrFile))
    {
      fclose(edrFile);
      
      fileIsOpen = false;

      msgsReadPerFile = 0;
    }
  
  return EdrReport::BAD_DATA;
}

///////////////////////////////////////////////////////////////
//
// FileInput::getFileTime: Set members year, month, day, hour
//                         minute, second which correspond to
//                         filetime.
//
int FileInput::getFileTime( char *filename)
{
 
  //
  // Get file time from filename: filename is */yyyymmdd/hhMMss.*
  //
  // locate the last slash in filename:
  // 
  int slash = '/';
  
  char *datePtr = strrchr(filename, slash);
      
  //
  // point at the date (yymmdd)
  //
  datePtr -= 8;
  
  sscanf(datePtr, "%4d%2d%2d%*c%2d%2d%2d%*4c", &year, &month, &day,
	 &hour, &min, &sec);

  return 0;
}










