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
///////////////////////////////////////////////////////////////
// TimestampFile.cc
//
// TimestampFile object
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2006
//
///////////////////////////////////////////////////////////////


#include <toolsa/DateTime.hh>
#include "TimestampFile.hh"

using namespace std;

// Constructor

TimestampFile::TimestampFile(int argc, char **argv) :
  _progName("TimestampFile"),
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
   
   
  // process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  return;

}

// destructor

TimestampFile::~TimestampFile()

{
  // unregister

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// 
// Run
//
int TimestampFile::Run()
{
  
  PMU_auto_register("TimestampFile::Run()");
  
  if (_params.debug)
    cerr << "TimestampFile::Run() \n";

  redirectStdin();
  
  return 0;
 
}


int TimestampFile::redirectStdin()
{

  char outputFile[256];
  
  char outfileName[64];

  char mkdirCommand[512];

  char datestr[9];

  char timestr[16];

  FILE *optr;
 
  struct timeval tv;

  struct timezone tz;

  DateTime filetime;

  if (_params.debug)
    {
      cerr << "TimestampFile::redirectStdin(): writing data to file." << " \n";
    }

  //
  // Get the time
  // 
  gettimeofday(&tv, &tz);
 
  filetime.set(tv.tv_sec);
 
  //
  // Create date and time strings (yyyymmdd, hhMMss.sssss)
  //
  sprintf(datestr, "%.4d%.2d%.2d", filetime.getYear(), filetime.getMonth(),  filetime.getDay());
  
  sprintf(timestr, "%.2d%.2d%.2d.%.6ld", filetime.getHour(), filetime.getMin(), filetime.getSec(),tv.tv_usec);


  //
  // Create output directory and filenames (*/yyyymmdd/hhMMss.sss._params.file_ext)
  //
  sprintf(outfileName, "%s.%s", timestr, _params.file_ext);
  
  sprintf( outputFile, "%s/%s/%s", _params.output_dir, datestr, outfileName);
  

  //
  // Make outputdir 
  //
  sprintf(mkdirCommand, "mkdir -p %s/%s",  _params.output_dir, datestr);
 
  if (_params.debug)
    {
      cerr << "TimestampFile::redirectStdin(): " <<  mkdirCommand << endl;
      
    }

  system( mkdirCommand);


  // 
  // Open output file
  //
  if ( (optr = fopen(outputFile, "w") ) == NULL)
    {
      cerr << "TimestampFile::redirectStdin(): ERROR opening output file " 
	   << outputFile << " \n";
      
      return 1;
    }

  char *buffer = new char[16384];
  
  while( !(feof(stdin)) )
    {
      //
      // Copy message to buffer
      //
      
      memset(buffer, 0, 16384);

      int bytes_read = fread( buffer,1,16384,stdin); 
      
      int fret = fwrite(buffer, 1,bytes_read, optr);
 
      if (_params.debug)
	cerr << "bytes read: " << bytes_read << " bytes written: " << fret << endl;
    }
  
  
  if (_params.debug)
    cerr << "TimestampFile::redirectStdin(): Finished writing to " << outputFile;
  
  //
  // Clean up
  //
  fclose (optr);

  delete buffer;

  if (_params.write_ldata_info)
    {
      //
      // Write LdataInfoFile
      //
      char *filenameBasePtr = strrchr((char*)outputFile, '/');
      
      LdataInfo ldata;
      ldata.setDir( _params.output_dir);
      ldata.setDataFileExt("bufr");
      ldata.setUserInfo1(filenameBasePtr);
      ldata.setUserInfo2("none");
      ldata.write(tv.tv_sec);
    }

  return 0;

}
