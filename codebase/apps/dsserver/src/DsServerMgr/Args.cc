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
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//   Paddy McCarthy Modified Sept 1998 to remove public data members.
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <toolsa/umisc.h>
#include <dsserver/DsLocator.hh>

#include "Args.hh"
using namespace std;

/**************************************************************
* Constructor
*/

Args::Args(int argc, char **argv)
{
    char tmp_str[BUFSIZ];

    // intialize

    _okay = true;
    _showUsage = false;
    _paramsFilePath = (char *)NULL;
    _appName = (char*) "DsServerMgr";
    
    TDRP_init_override(&_override);

    //
    // Immediate override of port from DsLocator defaults
    //
    int port = DsLocator.getDefaultPort( _appName );
    if ( port != -1 ) {
       sprintf(tmp_str, "port = %d;", port);
       TDRP_add_override(&_override, tmp_str);
    }

    // loop through args
    bool thisArgPreprocessed = false;
    for (int i =  1; i < argc; i++) {
        if (thisArgPreprocessed) {
            // Do nothing.
            thisArgPreprocessed = false;
        }
        else if (strcmp(argv[i], "--") == 0 ||
                 strcmp(argv[i], "-h") == 0 ||
                 strcmp(argv[i], "-help") == 0 ||
                 strcmp(argv[i], "-man") == 0) {
            _showUsage = true;
            return;
        }
        else if (strcmp(argv[i], "-debug") == 0) {
            sprintf(tmp_str, "debug = DEBUG_NORM;");
            TDRP_add_override(&_override, tmp_str);
        }
        else if (strcmp(argv[i], "-verbose") == 0) {
            sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
            TDRP_add_override(&_override, tmp_str);
        }
	else if (!strcmp(argv[i], "-noThreads")) {
	  sprintf(tmp_str, "noThreads = TRUE;");
	  TDRP_add_override(&_override, tmp_str);
	}
	else if (!strcmp(argv[i], "-secure")) {
	  sprintf(tmp_str, "runSecure = TRUE;");
	  TDRP_add_override(&_override, tmp_str);
	}
	else if (!strcmp(argv[i], "-mdvReadOnly")) {
	  sprintf(tmp_str, "mdvReadOnly = TRUE;");
	  TDRP_add_override(&_override, tmp_str);
	}
	else if (!strcmp(argv[i], "-spdbReadOnly")) {
	  sprintf(tmp_str, "spdbReadOnly = TRUE;");
	  TDRP_add_override(&_override, tmp_str);
	}
        else if (strcmp(argv[i], "-instance") == 0) {
            if (i < argc - 1) {
                sprintf(tmp_str, "instance = %s;", argv[i + 1]);
                TDRP_add_override(&_override, tmp_str);
                thisArgPreprocessed = true;
            }
            else {
                _okay = false;
            }
        }
        else if (strcmp(argv[i], "-params") == 0) {
            if (i < argc - 1) {
                _paramsFilePath = argv[i + 1];
                thisArgPreprocessed = true;
            }
            else {
                _okay = false;
            }
        } 
        else if (strcmp(argv[i], "-qmax") == 0) {
            if (i < argc - 1) {
                char * result;
    
                int qmax = (int) strtol(argv[i + 1], &result, 10);
                if (*result != '\0') {  
                    cerr << "Error: Bad characters in qmax specification: "
                         << argv[i + 1] << endl;
                    cerr << "First bad char: \"" << *result << "\"." << endl;
                    cerr << "Parsed string: \"" << argv[i + 1] << "\"." << endl;
                }
    
                sprintf(tmp_str, "maxQuiescentSecs = %d;", qmax);
                TDRP_add_override(&_override, tmp_str);
    
                thisArgPreprocessed = true;
            }
            else {
                _okay = false;
                cerr << "Error: No qmax value specified." << endl;
                continue;
            }
        } 
        else if (strcmp(argv[i], "-port") == 0) {
            if (i < argc - 1) {
                char * result;
                int port = (int) strtol(argv[i + 1], &result, 10);
    
                if (*result != '\0') {  
                    cerr << "Error: Bad characters in port specification: "
                         << argv[i + 1] << endl;
                    cerr << "First bad char: \"" << *result << "\"." << endl;
                    cerr << "Parsed string: \"" << argv[i + 1] << "\"." << endl;
                }
    
                sprintf(tmp_str, "port = %d;", port);
                TDRP_add_override(&_override, tmp_str);
    
                thisArgPreprocessed = true;
            }
            else {
                _okay = false;
                cerr << "Error: No port value specified." << endl;
                continue;
            }
        }
        else {
            // Unknown arg -- ignore.
            cerr << "Unknown arg: " << argv[i] << endl;
        }
    } // i

    if (!_okay) {
        _showUsage = true;
    }
}

/**************************************************************
* protected methods
**************************************************************/

/**************************************************************
* usage()
*/

void Args::usage(FILE *out)
{
  fprintf(out, "%s%s%s%s",
	  "Usage: ", _appName, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -debug ] print debug error messages\n"
	  "       [ -instance ? ] instance (default primary)\n"
	  "       [ -noThreads ] force single-threaded operation.\n"
	  "       [ -port ? ] port (default 5434)\n"
	  "       [ -qmax ? ] qmax (secs) (default 3600)\n"
	  "       [ -mdvRdOnly ] start DsMdvServer in read-only mode.\n"
	  "       [ -spdbRdOnly ] start DsSpdbServer in read-only mode.\n"
	  "       [ -secure ] starts servers in secure mode.\n"
	  "         No puts to absolute paths, i.e. starting with '/'\n"
	  "                 or paths containing '..'\n"
	  "       [ -verbose ] print verbose messages\n"
	  "\n");
  
    TDRP_usage(out);
}

