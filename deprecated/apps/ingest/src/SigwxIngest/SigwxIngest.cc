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

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include "SigwxIngest.hh"
#include "BufrSigwxProcessor.hh"
using namespace std;

// Constructor

SigwxIngest::SigwxIngest(int argc, char **argv) :
  _progName("SigwxIngest"),
  _args(_progName)
  
{
  
  isOK = TRUE;
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with command line args." << endl;
    isOK = FALSE;
    return;
  }
  
  // get TDRP params
  
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    isOK = FALSE;
    return;
  }

  //
  // Initialize file trigger
  //
  if ( _params.mode == Params::REALTIME ) 
    {
      if (_params.debug)
	cerr << "FileInput::init: Initializing realtime input from " 
	     << _params.input_dir << endl;
      
      fileTrigger = new DsInputPath( _progName, _params.debug,
				     _params.input_dir,
				     _params.max_valid_age_min*60,
				     PMU_auto_register,
				     _params.ldata_info_avail,
				     false );
      //
      // Set wait for file to be written to disk before being served out.
      //
      fileTrigger->setFileQuiescence( _params.file_quiescence_sec );
      
      //
      // Set time to wait before looking for new files.
      // 
      fileTrigger->setDirScanSleep( _params.check_input_sec );  
    }

   if ( _params.mode == Params::FILELIST ) 
    {    
      //
      // Archive mode.
      //
      const vector<string> &fileList =  _args.getInputFileList();
      if ( fileList.size() ) 
	{
	  if (_params.debug)
	    cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;
	  
	  fileTrigger = new DsInputPath( _progName, _params.debug , fileList );
	}
    }

   if ( _params.mode == Params::TIME_INTERVAL ) 
     { 
       //
       // Set up the time interval
       //
       DateTime start( _params.start_time);
      
       DateTime end( _params.end_time);
       
       time_t time_begin = start.utime();
       
       time_t time_end = end.utime();
       
       if (_params.debug)
	 {
	   cerr << "FileInput::init: Initializing file input for time interval [" 
		<< time_begin << " , " << time_end << "]." << endl;
	   
	 }
       
       fileTrigger = new DsInputPath( _progName, _params.debug,
				      _params.input_dir,
				      time_begin,
				      time_end);
       

     }
   
   
   
  // process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  return;

}

// destructor

SigwxIngest::~SigwxIngest()

{
  // unregister

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// 
// Run
//
int SigwxIngest::Run()
{
  
  PMU_auto_register("SigwxIngest::Run(): Reading message");
  
  if (_params.debug)
    cerr << "SigwxIngest::Run() \n";

  
  bool gotData;
      
  char * newfile = fileTrigger->next();
  
  if (!newfile)
    gotData = false;
  else
    gotData = true;
  
  while (gotData)
    {
      processFile(newfile);
      PMU_auto_register("SigwxIngest::Run():Checking for data.");
      newfile = fileTrigger->next();
      
      if (!newfile)
	gotData = false;
      
    }
  
  return 0;
 
}

int SigwxIngest::processFile(char *inputFile)
{
 
   FILE *fptr;

   if (_params.debug)
   {
     cerr << "SigwxIngest::processFile(): processing " << inputFile << " \n";
   }

   PMU_auto_register("SigwxIngest::processFile(): Separating each BUFR message and processing");

   if ( (fptr = fopen(inputFile, "r") ) == NULL)
   {
     cerr << "SigwxIngest::processFile(): ERROR opening " << inputFile << " \n";

     return 1;
   }
   
   PMU_auto_register("SigwxIngest::processFile(): Doing just that."); 
   
   _findSigwxReports(fptr);
   
   int msgStart = -1;
   
   int msgEnd = -1;
   
   int count = 0;
   
   for (int i = 0; i < _sigwxReps.size(); i++)
   {
     fseek(fptr, _sigwxReps[i].first, SEEK_SET);
     
     int n = ftell (fptr);
     while ( n <  _sigwxReps[i].second && !(feof(fptr)) )
     {
       
       int c = fgetc(fptr);
       //
       // Check and see if the character is a B, then check for U, F, R.
       // If we get BUFR, mark the start of the message.
       //
       if (c == 'B')
       {
	 c = fgetc(fptr);
	 
	 if (c == 'U')
	 {
	   c = fgetc(fptr);
	   
	   if (c == 'F')
	   {
	     c = fgetc(fptr);
	     
	     if (c == 'R')
	     {
	       //
	       // Get the location of the start of the message
	       // ftell points to the byte after 'R', so go back 4 bytes to the 'B'
	       //
	       msgStart = ftell(fptr);

	       msgStart = msgStart - 4;

	       if (_params.debug)
	       {
		 cerr << "SigwxIngest::processFile(): msgStart is " << msgStart << endl;
	       }
	     }
	   }
	 }
       }
       //
       // Check and see if the character is a 7, then check for 7, 7, 7.
       // If we get 7777, mark the end of the message.
       //
       if (c == '7')
       {
	 c = fgetc(fptr);
	 
	 if (c == '7')
	 {
	   c = fgetc(fptr);
	   
	   if (c == '7')
	   {
	     c = fgetc(fptr);
	     
	     if (c == '7')
	     {
	       if (_params.debug == Params::DEBUG_VERBOSE)
		 cerr << "SigwxIngest::parseFile: Ending sequence of the BUFR messge found.\n";
	       
	       msgEnd = ftell(fptr);
	       
	       if (_params.debug)
		 cerr << "SigwxIngest::processFile(): msgEnd is " << msgEnd << endl;
	     }
	   }
	 }
       }
       
       //
       // If both the start and end offsets have been set, write the message to an output
       // file.
       //
       if (msgStart > -1 && msgEnd > -1 && (msgStart <= msgEnd))
       {	
	 //
	 // Copy message to buffer
	 //
	 char *buffer = new char[16384];
	 
	 memset(buffer, 0, 16384);
	  
	 fseek(fptr,msgStart, SEEK_SET);
	 
	 fread(buffer,1, msgEnd - msgStart + 1,fptr);
	 
	 if (_params.debug)
	   cerr << "SigwxIngest::processFile(): Creating BUFR processor\n";
	 
	 PMU_auto_register("SigwxIngest::parseFile(): Creating BUFR processor");
	 
	 FILE *optr;
	 
	 string outfileName("/tmp/bufr.tmp");
	 
	 if ( (optr = fopen(outfileName.c_str(),"w") ) == NULL)
	 {
	   cerr << "SigwxIngest::processFile(): ERROR opening output file "
		<< outfileName.c_str() << " \n";
	   return 1;
	 }
	 
	 fwrite(buffer, 1, msgEnd - msgStart + 1, optr);
	 
	 fclose(optr);
	 
	 BufrSigwxProcessor bufrSigwxProcessor((char*)outfileName.c_str(), 
					       _params.output_url, _params.debug);
	  
	 bufrSigwxProcessor.process();

	 n =  ftell(fptr);

	 msgStart = -1;
	 
	 msgEnd = -1;
	 
	 delete buffer;
       }
     }// end while    
   } // end for 
   
   if(_params.debug)
   {
     cerr << "SigwxIngest::processFile(): Closed " << inputFile << ". Processing complete." << endl;
   }

   if(fptr)
   {
     fclose (fptr);
   }
   
   return 0;
}

int SigwxIngest::_findSigwxReports(FILE *fptr)
{
  //
  // Find the beginning of the next message and its length and copy
  // it to an output file of the same name
  // 
  int msgStart = -1;
  
  int msgEnd = -1;
  
  int count = 0;

  rewind(fptr);
  
  while( !(feof(fptr)) )
  {   
    int c = fgetc(fptr);
    //
    // Check and see if we have a "####" string to mark the start
    // of a SIGWX report
    //
    if (c == '#')
    {
      c = fgetc(fptr);
      
      if (c == '#')
      {
	c = fgetc(fptr);
	
	if (c == '#')
	{
	  c = fgetc(fptr);
	  
	  if (c == '#')
	  {
	    if (_params.debug == Params::DEBUG_VERBOSE)
	      cerr << "SigwxIngest::parseFile: Found start of a BUFR messge!\n";
	    
	    //
	    // We have the beginning of a msg. We may want to mark the end
	    // of another.
	    //
	    if( msgStart > 0)
	    {
	      msgEnd = ftell(fptr);
	      
	      //
	      // Back up 4 spaces for the '####'
	      //
	      msgEnd = msgEnd - 4;

	      //
	      // Store the start end pair
	      //
	      pair <long,long> msgStartEnd;

	      msgStartEnd.first =  msgStart;

	      msgStartEnd.second = msgEnd;
	      
	      cerr << "Msg Start: " << msgStart << endl;

	      cerr << "Msg End: " << msgEnd << endl;

	      _sigwxReps.push_back(msgStartEnd);

	      //
	      // Reset msg start and end flags
	      //
	      msgEnd = -1;

	      msgStart = -1;
	    }
	    
	    char line[18];

	    fgets(line,18,fptr);

	    cerr << "start of msg: " << line;
	    
	    fgets(line,18,fptr);

	    cerr << "msg type : " << line << endl << endl;
	    if ( strncmp(line,"JUB",3) == 0  ||  strncmp(line,"JUV",3)==0  )
	    {
	      cerr << "We have cloud or storm." << endl;
	     
	      msgStart = ftell(fptr);
	    } 
	  }
	}
      }
    }    
  }// end while
  
  if (msgStart > 0)
  {
    //
    // End of file must be the end of message
    //
    fseek(fptr, 1 ,SEEK_END);
    
    msgEnd = ftell(fptr);
  }

  for (int i = 0; i < (int) _sigwxReps.size(); i++)
  {
    cerr << "Report file start: " << _sigwxReps[i].first 
	 << " Report file end: " << _sigwxReps[i].second << endl;
  }

  if (_sigwxReps.size() == 0)
  {
    cerr << "SigwxIngest::_findSigwxReports(): No cloud or storm reports found" << endl;
  }

  rewind(fptr);

  return 0;
}
