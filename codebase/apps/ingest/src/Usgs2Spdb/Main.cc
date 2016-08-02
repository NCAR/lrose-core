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
//
// main for Usgs2Spdb
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2006
//
///////////////////////////////////////////////////////////////
//
// Usgs2Spdb reads USGS data from argc or an ASCII file, converts them to
// usgsData_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#include <tdrp/tdrp.h>
#include "Usgs2Spdb.hh"
#include "Params.hh"

using namespace std;

void usage(char *prog_name)
{
  cerr << "Usage: \n";
  Params::usage(cerr);
  cerr << " options:\n";
  cerr << "    [ -h, -help ] produce this list.\n";
  cerr << "    [ -debug or -verbose ] print debug messages\n";
  cerr << " " << prog_name << " -file <file>\n";
  cerr << " " << prog_name << " -fileList <file1> <file2> <file3> ...\n";
  cerr << " " << prog_name << " -Volcano <title> <sender> <lat> <lon> <summit> <code> <time> <id>\n";
  cerr << " " << prog_name << " -Earthquake <title> <id> <sender> <version> <magnitude> <magnitudeType> \n";
  cerr << "            <time> <lat> <lon> <depth> <horizontalError> <verticalerror>\n";
  cerr << "            <statsion> <phases> <minDistance> <RMSError> <azimuthalGap>\n";

}

int main(int argc, char **argv)
{
  char tmp_str[BUFSIZ];
  char *paramsFilePath;
  tdrp_override_t override;
  TDRP_init_override(&override); 

  for(int a = 0; a < argc; a++) {
    if(argv[a] == "-help" || argv[a] == "-h" || argv[a] == "--help" || argv[a] == "--h") {
      usage(argv[0]);
      return -2;
    } else if (!strcmp(argv[a], "-debug")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[a], "-verbose")) {
      
      sprintf(tmp_str, "debug = TRUE;");
      TDRP_add_override(&override, tmp_str);
    }
  }

  // Get the TDRP parameters.
  Params P;
  if (P.loadFromArgs(argc, argv, override.list, &paramsFilePath)) {
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 
     
  TDRP_free_override(&override);

  Usgs2Spdb usgs2spdb(&P);      
  int ret;
  for(int a = 1; a < argc; a++) {
    if(strcmp(argv[a], "-fileList") == 0 || strcmp(argv[a], "-file") == 0 ) {
      if (P.debug == Params::DEBUG_VERBOSE)
	cout << "Running in fileList mode\n";
      for(int b = a+1; b < argc; b++) {
	ret = usgs2spdb.processFile(argv[b]);
	if(ret != 0)
	  return ret;
      }
      return 0;
    } else if(strcmp(argv[a], "-Volcano") == 0) {
      if(argc < a + 10) {
	usage(argv[0]);
	return -2;
      } else {
	if (P.debug == Params::DEBUG_VERBOSE)
	  cout << "Running in Volcano command line argument mode\n";
	return usgs2spdb.saveVolcano(argv[a+1], argv[a+2], argv[a+3], argv[a+4], 
				     argv[a+5], argv[a+6], argv[a+7], 
				     argv[a+8], argv[a+9]);
      }
    } else if(strcmp(argv[a], "-Earthquake") == 0)
      if(argc < a + 19) {
	usage(argv[0]);
	return -2;
      } else {
	if (P.debug == Params::DEBUG_VERBOSE)
	  cout << "Running in Earthquake command line argument mode\n";
	return usgs2spdb.saveEarthquake(argv[a+1], argv[a+2], argv[a+3], argv[a+4], argv[a+5], 
					argv[a+6], argv[a+7], argv[a+8], argv[a+9], argv[a+10], 
					argv[a+11], argv[a+12], argv[a+13], argv[a+14],
					argv[a+15], argv[a+16], argv[a+17], argv[a+18]);
      }
  }

  usage(argv[0]);
  return -2;
}
