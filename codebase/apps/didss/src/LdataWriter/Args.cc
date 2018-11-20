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
//////////////////////////////////////////////////////////
// Args.cc
//
// Command line args
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <toolsa/umisc.h>
#include <ctime>
using namespace std;

// constructor

Args::Args()

{
  debug = false;
  verbose = false;
  latestTime = time(NULL);
  isFcast = false;
  leadTime = 0;
  refresh = false;
  maxDataTime = false;
  startTime = 0;
  endTime = 0;
  msecsSleep = 100;
}

// destructor

Args::~Args()

{

}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      debug = true;
      verbose = true;
      
    } else if (!strcmp(argv[i], "-dir")) {
      
      if (i < argc - 1) {
	dir = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -dir" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	url = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -url" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-dtype")) {
      
      if (i < argc - 1) {
	dataType = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -dtype" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ext")) {
      
      if (i < argc - 1) {
	fileExt = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -ext" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-rpath")) {
      
      if (i < argc - 1) {
	relDataPath = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -rpath" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-info1")) {
      
      if (i < argc - 1) {
	userInfo1 = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -info1" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-info2")) {
      
      if (i < argc - 1) {
	userInfo2 = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -info2" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ltime")) {
      
      if (i < argc - 1) {
	date_time_t ltime;
	if (sscanf(argv[++i],
		   "%04d%02d%02d%02d%02d%02d",
		   &ltime.year, &ltime.month, &ltime.day,
		   &ltime.hour, &ltime.min, &ltime.sec) == 6) {
	  uconvert_to_utime(&ltime);
	  latestTime = ltime.unix_time;
	} else {
          cerr << "ERROR parsing command line arg: -ltime" << endl;
	  iret = -1;
	}
      } else {
        cerr << "ERROR parsing command line arg: -ltime" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-lead")) {
      
      if (i < argc - 1) {
	int lead_time;
	if (sscanf(argv[++i], "%d", &lead_time) == 1) {
	  leadTime = lead_time;
	  isFcast = true;
	} else {
	  iret = -1;
          cerr << "ERROR parsing command line arg: -lead" << endl;
	}
      } else {
	iret = -1;
        cerr << "ERROR parsing command line arg: -lead" << endl;
      }

    } else if (!strcmp(argv[i], "-writer")) {
      
      if (i < argc - 1) {
	writer = argv[++i];
      } else {
        cerr << "ERROR parsing command line arg: -writed" << endl;
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-refresh")) {
      
      refresh = true;
      
    } else if (!strcmp(argv[i], "-displacedDataDir")) {

       if( i < argc -1) {
         displacedDataDir = argv[++i];
       } else {
         cerr << "ERROR parsing command line arg: -displacedDataDir" << endl;
         iret = -1;
       }

    } else if (!strcmp(argv[i], "-maxDataTime" )) {

       maxDataTime = true;
    
    } else if (!strcmp(argv[i], "-startTime")) {
      
      if (i < argc - 1) {
	date_time_t start;
	if (sscanf(argv[++i],
                   "%04d%02d%02d%02d%02d%02d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
          cerr << "ERROR parsing command line arg: -startTime" << endl;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	}
      } else {
	iret = -1;
        cerr << "ERROR parsing command line arg: -startTime" << endl;
      }
	
    } else if (!strcmp(argv[i], "-endTime")) {
      
      if (i < argc - 1) {
	date_time_t end;
	if (sscanf(argv[++i],
                   "%04d%02d%02d%02d%02d%02d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
          cerr << "ERROR parsing command line arg: -endTime" << endl;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	}
      } else {
	iret = -1;
        cerr << "ERROR parsing command line arg: -endTime" << endl;
      }
	
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    inputFileList.push_back(argv[j]);
	  }
	}
      } else {
	iret = -1;
      }

      if (inputFileList.size() < 1) {
        cerr << "ERROR parsing command line arg: -f" << endl;
      }

    } else if (!strcmp(argv[i], "-sleepMsecs")) {
      
      if (i < argc - 1) {
	int val;
	if (sscanf(argv[++i], "%d", &val) == 1) {
	  msecsSleep = val;
	} else {
	  iret = -1;
          cerr << "ERROR parsing command line arg: -sleepMsecs" << endl;
	}
      } else {
	iret = -1;
        cerr << "ERROR parsing command line arg: -sleepMsecs" << endl;
      }

    } // if
    
  } // i
  
  if( maxDataTime ) {
     if( userInfo2.size() > 0 ) {
        userInfo2 += "Using max data time";
     }
     else {
        userInfo2 = "Using max data time";
     }
  }

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << "LdataWriter allows you to write a latest_data_info file\n"
      << "  from a script, by specifying the details on the\n"
      << "  command line.\n"
      << endl;

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -dir ? ] data directory\n"
      << "          Dir is relative to $RAP_DATA_DIR unless it\n"
      << "          starts with . or /\n"
      << "       [ -displacedDataDir ?] path for directory if it does not\n"
      << "         reside with the latest data info file\n"
      << "       [ -dtype ? ] data type - for DataMapper\n"
      << "       [ -endTime YYYYMMDDHHMMSS ] optional end time for -refresh\n"
      << "         Reprocesses files before this time and after start time\n"
      << "       [ -ext ? ] file extension\n"
      << "       [ -f ? ? ? ] optional file lit to -refresh\n"
      << "       [ -info1 ? ] user info 1\n"
      << "       [ -info2 ? ] user info 2\n"
      << "       [ -lead ? ] lead time in secs\n"
      << "         Multiple lead times may be specified, in order\n"
      << "       [ -ltime YYYYMMDDHHMMSS ] latest data time\n"
      << "          If ltime not specified, current time is used\n"
      << "       [ -maxDataTime ] if set, the latest data file will only\n"
      << "         be updated if the data time passed in is later than the\n"
      << "         data time read from the latest data info file\n"
      << "       [ -refresh ] refresh the current latest_data_info file\n"
      << "         reads latest entry, writes it again to refresh the info\n"
      << "         Can also be run on a series of files. See -f,\n"
      << "         -startTime, -endTime and -sleepMsecs\n"
      << "       [ -rpath ? ] data path relative to dir\n"
      << "       [ -sleepMsecs ? ] millisecs to sleep between files in -refresh mode\n"
      << "         Applies if using -f or -startTime/-endTime\n"
      << "         Default is 100 msecs\n"
      << "       [ -startTime YYYYMMDDHHMMSS ] optional start time for -refresh\n"
      << "         Reprocesses files after this time and before end time\n"
      << "       [ -url ? ] URL for remote writes\n"
      << "          e.g. ldatap:://hostname::mydata/set\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -writer ? ] name of writer application\n"
      << endl;
  
}



















