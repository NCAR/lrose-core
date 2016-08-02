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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2015
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

Args::Args ()

{
  TDRP_init_override(&override);
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[256];

  // intialize

  validTime = time(NULL);
  inputFileList.clear();
  vals.clear();
  tags.clear();
  xml.clear();

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-d") ||
               !strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-v") ||
               !strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-vv") ||
               !strcmp(argv[i], "-extra")) {
      
      sprintf(tmp_str, "debug = DEBUG_EXTRA;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "mode = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-datatype")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "datatype = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-datatype2")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "datatype2 = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-expire_secs")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "expire_secs = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-url")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "output_url = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-outer_tag")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "outer_xml_tag = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-spdb_label")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "spdb_label = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-spdb_id")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "spdb_id = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-delim")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "column_delimiter = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-comment")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "comment_character = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-tag")) {
      
      if (i < argc - 1) {
	tags.push_back(argv[++i]);
        sprintf(tmp_str, "mode = COMMAND_LINE;");
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      xml.clear();
	
    } else if (!strcmp(argv[i], "-val")) {
      
      if (i < argc - 1) {
	vals.push_back(argv[++i]);
        sprintf(tmp_str, "mode = COMMAND_LINE;");
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      xml.clear();
	
    } else if (!strcmp(argv[i], "-xml")) {
      
      if (i < argc - 1) {
	xml = argv[++i];
        sprintf(tmp_str, "mode = COMMAND_LINE;");
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      tags.clear();
      vals.clear();
	
    } else if (!strcmp(argv[i], "-valid")) {
      
      if (i < argc - 1) {
	int year, month, day, hour, min, sec;
	if (sscanf(argv[++i], "%d %d %d %d %d %d",
		   &year, &month, &day,
		   &hour, &min, &sec) != 6) {
	  iret = -1;
        }
        DateTime vtime(year, month, day, hour, min, sec);
        validTime = vtime.utime();
        sprintf(tmp_str, "mode = COMMAND_LINE;");
        TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
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
        sprintf(tmp_str, "mode = FILELIST;");
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
      
    } // if

  } // i

  if (vals.size() != tags.size()) {
    cerr << "ERROR - " << prog_name << endl;
    cerr << "  If you specify tags and vals in command line mode, you must" << endl;
    cerr << "  specify the same number of each." << endl;
    cerr << "  N vals specified: " << vals.size() << endl;
    cerr << "  N tags specified: " << tags.size() << endl;
    iret = -1;
  }

  if (iret) {
    usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::usage(string &prog_name, ostream &out)
{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -comment ] specify comment characted, default is '#'\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -datatype ? ] specify datatype for SPDB\n"
      << "         If numeric it is used directly\n"
      << "         If non-numeric the first 4 chars are hashed into an int\n"
      << "       [ -datatype2 ? ] specify datatype2 for SPDB\n"
      << "         If numeric it is used directly\n"
      << "         If non-numeric the first 4 chars are hashed into an int\n"
      << "       [ -delim ] specify column delimiter, default is ','\n"
      << "       [ -expire_secs ? ] set number of secs before expiry of product\n"
      << "                          after valid time\n"
      << "       [ -f ? ] specify file paths - can be multiple\n"
      << "         Sets mode to FILELIST\n"
      << "       [ -mode ?] FILELIST or COMMAND_LINE\n"
      << "       [ -spdb_id ? ] specify SPDB integer ID (default is 55)\n"
      << "       [ -spdb_label ? ] specify SPDB string label (default is 'XML data')\n"
      << "       [ -outer_tag ? ] specify outer XML tag\n"
      << "       [ -tag ? ] add a tag to the list\n"
      << "         Multiple tags are specified by multiple usage of -tag\n"
      << "         Sets mode to COMMAND_LINE\n"
      << "       [ -val ? ] add a value to the list\n"
      << "         Multiple values are specified by multiple usage of -val\n"
      << "         Sets mode to COMMAND_LINE\n"
      << "       [ -url ? ] specify output URL\n"
      << "       [ -valid \"yyyy mm dd hh mm ss\"] set valid time\n"
      << "         Sets mode to COMMAND_LINE\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -vv, -extra ] print extra verbose debug messages\n"
      << "       [ -xml ? ] specify the XML string\n"
      << "         Only a single XML string can be specified\n"
      << "         Sets mode to COMMAND_LINE\n"
      << endl;
  
  Params::usage(out);

}

