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
// Args.cc : command line args
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include <Spdb/Spdb.hh>
#include <didss/DsURL.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <cstring>
#include <string>
using namespace std;

// Constructor

Args::Args()

{
}

// Destructor

Args::~Args()

{
}

////////////////////////////////////////////////////////////////
// parse the command line - returns 0 on success, -1 on failure

int Args::parse(int argc, char **argv, string &prog_name)

{

  // intialize

  int iret = 0;
  bool urlSet = false;

  _progName = prog_name;
  debug = false;
  verbose = false;
  noHeader = false;
  blankLine = false;
  timeLabel = false;
  mode = intervalMode;
  refsOnly = false;
  unique = uniqueOff;
  requestTime = time(NULL);
  endTime = requestTime;
  startTime = endTime - 86400;
  timeMargin = 86400;
  dataTypes.clear();
  dataTypes.push_back(0);
  dataType2 = 0;
  doLastN = -1;
  doReverse = false;
  respectZeroTypes = false;
  horizLimitsSet = false;
  vertLimitsSet = false;
  timeListMinInterval = 1;
  threaded = false;
  checkWriteTimeOnGet = false;
  compressDataBufOnGet = false;
  printAsXml = false;
  printXmlHeaders = false;
  latestWriteTime = 0;

  // get URL, check validity

  if (argc < 2) {
    cerr << "Must put URL on command line." << endl;
    _usage(cerr);
    return (-1);
  }

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(cout);
      exit(0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      debug = true;
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      debug = true;
      verbose = true;
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "exact")) {
	  mode = exactMode;
	} else if (!strcmp(modestr, "closest")) {
	  mode = closestMode;
	} else if (!strcmp(modestr, "interval")) {
	  mode = intervalMode;
	} else if (!strcmp(modestr, "valid")) {
	  mode = validMode;
	} else if (!strcmp(modestr, "latest")) {
	  mode = latestMode;
	} else if (!strcmp(modestr, "before")) {
	  mode = firstBeforeMode;
	} else if (!strcmp(modestr, "after")) {
	  mode = firstAfterMode;
	} else if (!strcmp(modestr, "times")) {
	  mode = timesMode;
	} else if (!strcmp(modestr, "header")) {
	  mode = headerMode;
	} else if (!strcmp(modestr, "timelist")) {
	  mode = timeListMode;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-unique")) {
      
      if (i < argc - 1) {
	char *modestr = argv[++i];
	if (!strcmp(modestr, "latest")) {
	  unique = uniqueLatest;
	} else if (!strcmp(modestr, "earliest")) {
	  unique = uniqueEarliest;
	} else {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-time")) {
      
      if (i < argc - 1) {
	date_time_t request;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &request.year, &request.month, &request.day,
		   &request.hour, &request.min, &request.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&request);
	  requestTime = request.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
	date_time_t start;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &start.year, &start.month, &start.day,
		   &start.hour, &start.min, &start.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&start);
	  startTime = start.unix_time;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
	date_time_t end;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &end.year, &end.month, &end.day,
		   &end.hour, &end.min, &end.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&end);
	  endTime = end.unix_time;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-margin")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &timeMargin) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-min_interval")) {
      
      if (i < argc - 1) {
	unsigned int minInt;
	if (sscanf(argv[++i], "%u", &minInt) != 1) {
	  iret = -1;
	}
	timeListMinInterval = minInt;
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	urlStr = argv[++i];
	urlSet = true;
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-data_type")) {
      
      if (i < argc - 1) {
	string dataTypeStr(argv[++i]);
	_setDataTypes(dataTypeStr);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-data_type2")) {
      
      if (i < argc - 1) {
	i++;
	char *first=strchr(argv[i], '\"');
	char *last=strrchr(argv[i], '\"');
	if (first != NULL && last != NULL) {
	  int len = last - first - 1;
	  char tmp[BUFSIZ];
	  memset(tmp, 0, sizeof(tmp));
	  memcpy(tmp, first+1, len);
	  if(strlen(tmp) == 4) {
		dataType2 = Spdb::hash4CharsToInt32(tmp);
	  } else {
		dataType2 = Spdb::hash5CharsToInt32(tmp);
	  }
        } else {
	  if (sscanf(argv[i], "%d", &dataType2) != 1) {
	    if(strlen(argv[i]) == 4) {
		  dataType2 = Spdb::hash4CharsToInt32(argv[i]);
	    } else {
		  dataType2 = Spdb::hash5CharsToInt32(argv[i]);
	    }
	  }
	} 
      } else {
	iret = -1;
      }
    
    } else if (!strcmp(argv[i], "-refs_only")) {

      refsOnly = true;
      noHeader = false;
      timeLabel = false;

    } else if (!strcmp(argv[i], "-no_header")) {

      noHeader = true;
      timeLabel = false;
      refsOnly = false;

    } else if (!strcmp(argv[i], "-blank_line")) {

      blankLine = true;

    } else if (!strcmp(argv[i], "-time_label")) {

      timeLabel = true;
      noHeader = false;
      refsOnly = false;
      
    } else if (!strcmp(argv[i], "-reverse")) {

      doReverse = true;

    } else if (!strcmp(argv[i], "-respect_zero")) {

      respectZeroTypes = true;

    } else if (!strcmp(argv[i], "-last_n")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &doLastN) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-hlimits")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%lg %lg %lg %lg",
		   &minLat, &minLon, &maxLat, &maxLon) != 4) {
	  iret = -1;
	} else {
	  horizLimitsSet = true;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-vlimits")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%lg %lg",
		   &minHt, &maxHt) != 2) {
	  iret = -1;
	} else {
	  vertLimitsSet = true;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-threaded")) {
      
      threaded = true;
      
    } else if (!strcmp(argv[i], "-lwrite")) {
      
      if (i < argc - 1) {
	date_time_t lwrite;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &lwrite.year, &lwrite.month, &lwrite.day,
		   &lwrite.hour, &lwrite.min, &lwrite.sec) != 6) {
	  iret = -1;
	} else {
	  uconvert_to_utime(&lwrite);
	  latestWriteTime = lwrite.unix_time;
          checkWriteTimeOnGet = true;
	}
      } else {
	iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-compress")) {
      
      compressDataBufOnGet = true;
      
    } else if (!strcmp(argv[i], "-as_xml")) {
      
      printAsXml = true;
      
    } else if (!strcmp(argv[i], "-xml_headers")) {
      
      printXmlHeaders = true;
      
    } else if (!strcmp(argv[i], "-xml")) {
      
      if (i < argc - 1) {
	auxXmlPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } // if
      
  } // i

  // check URL

  if (!urlSet) {
    cerr << endl << "ERROR - you must specify url." << endl;
    iret = -1;
  }

  if (urlSet) {
    DsURL url(urlStr);
    if (!url.isValid()) {
      cerr << "Invalid URL: " << urlStr << endl;
      cerr << url.getErrString() << endl;
      iret = -1;
    }
  }
  
  if (iret) {
    _usage(cerr);
    return (-1);
  }

  return (0);
    
}

void Args::_setDataTypes(const string &dataTypeStr)

{

  dataTypes.clear();
  
  // find the data_types - it can be a comma-delimited list

  string thisDataType;
  size_t startPos = 0;
  size_t commaPos = dataTypeStr.find(',', startPos);
  if (commaPos == string::npos) {
    thisDataType = dataTypeStr;
  } else {
    thisDataType.assign(dataTypeStr, startPos, commaPos - startPos);
  }
  
  while (thisDataType.size() != 0) {
    
    int dataType;
    if (thisDataType == "all") {
      dataType = 0;
    } else {
      const char *first=strchr(thisDataType.c_str(), '\"');
      const char *last=strrchr(thisDataType.c_str(), '\"');
      if (first != NULL && last != NULL) {
	int len = last - first - 1;
	char tmp[BUFSIZ];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, first+1, len);
	if(strlen(tmp) == 4) {
		dataType = Spdb::hash4CharsToInt32(tmp);
	} else {
		dataType = Spdb::hash5CharsToInt32(tmp);
	}
      } else {
	if (sscanf(thisDataType.c_str(), "%d", &dataType) != 1) {
	  if(thisDataType.size() == 4) {
		dataType = Spdb::hash4CharsToInt32(thisDataType.c_str());
	  } else {
		dataType = Spdb::hash5CharsToInt32(thisDataType.c_str());
	  }
	}
      }
    }
    dataTypes.push_back(dataType);

    if (commaPos == string::npos) {
      thisDataType = "";
    } else {
      startPos = commaPos + 1;
      commaPos = dataTypeStr.find(',', startPos);
      thisDataType.assign(dataTypeStr, startPos, commaPos - startPos);
    }

  } // while

}


void Args::_usage(ostream &out)
{
  
  out << "\n"
      << "Usage: " << _progName << " [args as below]\n"
      << "\n"
      << "Required:\n"
      << "\n"
      << "  [ -url ? ] specify URL\n"
      << "     URL format: spdbp:://host:port:directory (port optional)\n"
      << "                   or\n"
      << "                 directory\n"
      << "\n"
      << "Options:\n"
      << "\n"
      << "  [ --, -h, -help, -man ] produce this list.\n"
      << "\n"
      << "  [ -as_xml ] print out as xml\n"
      << "     Instead of normal output, print out as xml\n"
      << "\n"
      << "  [ -compress ] set compression for get messages\n"
      << "     If a server is contacted, the data part of the return message\n"
      << "     will be compressed\n"
      << "\n"
      << "  [ -debug ] print debug messages.\n"
      << "\n"
      << "  [ -verbose ] print verbose debug messages.\n"
      << "\n"
      << "  [ -mode ? ] get mode\n"
      << "     options: interval (default), exact, closest, valid,\n"
      << "              latest, before, after, times, header, timelist\n"
      << "     Note: header is for local request only\n"
      << "\n"
      << "  [ -time ? ] request_time: defaults to now\n"
      << "     modes: exact, closest, before, after, valid, header\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -start ? ] start_time: defaults to (now - 1 day)\n"
      << "     mode: interval, timelist\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -end ? ] end_time: defaults to now\n"
      << "     mode: interval, timelist\n"
      << "     format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -margin ? ] time_margin (secs): defaults to 1 day\n"
      << "     modes: closest, latest, before, after\n"
      << "\n"
      << "  [ -min_interval ? ] timelist min interval (secs): defaults to 1\n"
      << "     modes: timelist\n"
      << "\n"
      << "  [ -unique ? ] get unique\n"
      << "     options: latest, earliest\n"
      << "\n"
      << "  [ -data_type ? ] set data type.\n"
      << "     Either specify int or string - string will be hashed into\n"
      << "     an int. Default is 0.\n"
      << "     If string begins with digit, enclose in double quotes. Make sure\n"
      << "       to escape the double quotes in the shell. e.g. \\\"01\\\"\n"
      << "     May also be a comma-delimited list, no spaces\n"
      << "\n"
      << "  [ -data_type2 ? ] set data type 2.\n"
      << "     Either specify int or string - string will be hashed into\n"
      << "     an int. Default is 0.\n"
      << "     If string begins with digit, enclose in double quotes. Make sure\n"
      << "       to escape the double quotes in the shell. e.g. \\\"01\\\"\n"
      << "\n"
      << "  [ -refs_only ] only get chunk refs, no data.\n"
      << "\n"
      << "  [ -time_label] print only valid time, instead of full header.\n"
      << "\n"
      << "  [ -no_header] do not print header information, only data.\n"
      << "\n"
      << "  [ -blank_line ] add a blank line separator between entries.\n"
      << "\n"
      << "  [ -last_n ? ] only print the last n items found.\n" 
      << "     Use -unique to get the last 1 item found.\n" 
      << "\n"
      << "  [ -reverse ] print in reverse order (newest first).\n" 
      << "\n"
      << "  [ -respect_zero ] respect zero data types.\n"
      << "                    Only applies to get functions.\n"
      << "\n"
      << "  [ -hlimits \"min_lat min_lon max_lat max_lon\" ] set horiz limits.\n"
      << "     Only applicable to queries to servers which support this\n"
      << "     feature e.g. Metar2Symprod.\n"
      << "\n"
      << "  [ -threaded ] use threading for testing\n"
      << "    Uses DsSpdbThreaded object instead of DsSpdb object\n"
      << "\n"
      << "  [ -vlimits \"min_ht max_ht\"] set vertical limits.\n"
      << "     Only applicable to queries to servers which support this\n"
      << "     feature.\n"
      << "\n"
      << "  [ -lwrite ? ] set latest write time on get\n"
      << "     Only data written before this time will be returned on get.\n"
      << "     Format: \"YYYY MM DD HH MM SS\"\n"
      << "\n"
      << "  [ -xml ? ] set path for auxiliary xml commands\n"
      << "     If set, will read in XML commands from the specified path\n"
      << "     and set the auxXml string on the Spdb object.\n"
      << "     The aux xml will be sent to the server as applicable.\n"
      << "\n"
      << "  [ -xml_headers] print chunk header information in XML.\n"
      << "                  Default is false. Use with -as_xml\n"
      << "\n"
      << endl;

}
