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
// June 2003
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <Spdb/Spdb.hh>
#include <toolsa/udatetime.h>
#include <toolsa/str.h>
using namespace std;

// constructor

Args::Args()

{
  TDRP_init_override(&override);
}

// destructor

Args::~Args()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, const string &prog_name)

{

  int iret = 0;
  char tmp_str[256];  

  // initialize

  dataTypes.clear();
  dataType2 = -1;
  weatherType="";
  weatherTypeSet=false;
  doValid=true;
  endTime = time(NULL);
  startTime = endTime - 86400;
  headerText="";
  headerTextSet=false;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-i")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "instance = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-valid")) {
      
      if (i < argc - 1) {
	date_time_t vtime;
	if ((sscanf(argv[++i], "%d %d %d %d %d %d",
		    &vtime.year, &vtime.month, &vtime.day,
		    &vtime.hour, &vtime.min, &vtime.sec) != 6) &&
	    (vtime.year != -1)) {
	  iret = -1;
	}

	else {
	  if (vtime.year == -1) {
	    vtime.unix_time=time(NULL);
	  } else {
	    uconvert_to_utime(&vtime);
	  }
	  uconvert_from_utime(&vtime);
	  sprintf(tmp_str,
		  "valid_time = { %.4d, %.2d, %.2d, %.2d, %.2d, %.2d };",
		  vtime.year, vtime.month, vtime.day,
		  vtime.hour, vtime.min, vtime.sec);
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
      
      sprintf(tmp_str, "follow_cidd = false;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-point")) {
      
      if (i < argc - 1) {
	double lat, lon;
	if (sscanf(argv[++i], "%lg %lg", &lat, &lon) != 2) {
	  iret = -1;
	} else {
	  sprintf(tmp_str,
		  "closest_point = { %g, %g };", lat, lon);
	  TDRP_add_override(&override, tmp_str);
	  sprintf(tmp_str, "apply_bounding_box = FALSE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-bound")) {
      
      if (i < argc - 1) {
	double minlat, minlon, maxlat, maxlon;
	if (sscanf(argv[++i], "%lg %lg %lg %lg",
		   &minlon, &maxlon, &minlat, &maxlat) != 4) {
	  iret = -1;
	} else {
	  sprintf(tmp_str,
		  "bounding_box = { %g, %g, %g, %g };",
		  minlon, maxlon, minlat, maxlat);
	  TDRP_add_override(&override, tmp_str);
	  sprintf(tmp_str, "apply_bounding_box = TRUE;");
	  TDRP_add_override(&override, tmp_str);
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "input_url = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
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
	  char tmp[len + 1];
	  memset(tmp, 0, sizeof(tmp));
	  memcpy(tmp, first+1, len);
	  dataType2 = Spdb::hash4CharsToInt32(tmp);
	} else {
	  if (sscanf(argv[i], "%d", &dataType2) != 1) {
	    dataType2 = Spdb::hash4CharsToInt32(argv[i]);
	  }
	}
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-header_text")) {
      
      if (i < argc - 1) {
	headerText=argv[++i];
	headerTextSet=true;
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-weather_type")) {
      
      if (i < argc - 1) {
	weatherType=argv[++i];
	weatherTypeSet=true;
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
	  doValid=false;

	  //	  sprintf(tmp_str, "follow_cidd = false;");
	  //	  TDRP_add_override(&override, tmp_str);
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
	  doValid=false;

	  //	  sprintf(tmp_str, "follow_cidd = false;");
	  //	  TDRP_add_override(&override, tmp_str);
        }
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-reverse")) {
      
      sprintf(tmp_str, "reverse = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } // if
    
  } // i

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
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
	char tmp[len + 1];
	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, first+1, len);
	dataType = Spdb::hash4CharsToInt32(tmp);
      } else {
	if (sscanf(thisDataType.c_str(), "%d", &dataType) != 1) {
	  dataType = Spdb::hash4CharsToInt32(thisDataType.c_str());
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

void Args::_usage(const string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -bound \"minlon maxlon minlat maxlat\"]\n"
      << "         specify bounding box\n"
      << "       [ -data_type ? ] set data type (a.k.a. source).\n"
      << "         Either specify int or string - string will be hashed into\n"
      << "         an int. Default is 0.\n"
      << "         If string begins with digit, enclose in double quotes. Make sure\n"
      << "         to escape the double quotes in the shell. e.g. \\\"01\\\"\n"
      << "         May also be a comma-delimited list, no spaces\n"
      << "       [ -data_type2 ? ] set data type 2 (a.k.a. id).\n"
      << "         Either specify int or string - string will be hashed into\n"
      << "         an int. Default is 0.\n"
      << "         If string begins with digit, enclose in double quotes. Make sure\n"
      << "          to escape the double quotes in the shell. e.g. \\\"01\\\"\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -end ? ] end time.\n"
      << "         Forces non-CIDD (run-once) mode. Cannot use with -valid.\n"
      << "         Must also specify -start\n"
      << "         format: \"YYYY MM DD HH MM SS\"\n"
      << "       [ -header_text ? ] text to put on top of output. Only valid in\n"
      << "         in non-CIDD (run-once) mode and if print_banner is TRUE in\n"
      << "         param file\n" 
      << "       [ -i ? ] instance\n"
      << "       [ -point \"lat lon\"] specify closest point\n"
      << "       [ -url ? ] input URL\n"
      << "       [ -reverse ? ] print items in reverse order\n"
      << "       [ -start ? ] start time.\n"
      << "         Forces non-CIDD (run-once) mode. Cannot use with -valid.\n"
      << "         Must also specify -end\n"
      << "         format: \"YYYY MM DD HH MM SS\"\n"
      << "       [ -valid \"yyyy mm dd hh mm ss\"] set valid time.\n"
      << "         Forces non-CIDD (run-once) mode. To use the current time\n" 
      << "         pass in -1 for yyyy and skip mm dd hh mm ss\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << "       [ -weather_type ? ] set weather type.\n"
      << "         wild card (*) allowed\n"
      << "\n"
      << endl;
  
  Params::usage(out);

}
