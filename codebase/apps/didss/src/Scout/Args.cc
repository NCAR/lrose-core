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
// Sept 1998
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
using namespace std;

// Constructor

Args::Args ()
{

}

// Parse

int Args::parse (const string &prog_name, int argc, char **argv)

{

  // intialize

  int iret = 0;
  TDRP_init_override(&override);
  
  // loop through args
  
  char tmp_str[BUFSIZ];

  for (int i =  1; i < argc; i++) {
    
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?"))
	 ) {
      
      _usage(prog_name, cout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {
      
      sprintf(tmp_str, "Debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-verbose")) {
      
      sprintf(tmp_str, "Debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-once")){

      sprintf(tmp_str, "OnceOnly = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-nolock")){

      sprintf(tmp_str, "Lock = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-report")){

      sprintf(tmp_str, "Report = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-delay")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "BetweenPassDelay = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-dirsleep")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "BetweenDirDelay = %s;", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }


    } else if (!strcmp(argv[i], "-instance")) {
      
      if (i<argc-1) {
	sprintf(tmp_str, "Instance = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-comp_ext")) {
      
      if (i < argc - 1) {
	sprintf(tmp_str, "CompressedExt = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      } else {
	iret = -1;
      }

    } else if (!strcmp(argv[i], "-params")) {

      i++;

    } else if (!strcmp(argv[i], "-nolock")) {

      i++;

    } else if (!Params::isArgValid(argv[i])) {

      iret = -1;
      
    }
    
  } // i
  
  if (iret) {
    _usage(prog_name, cerr);
  }

  return iret;
    
}

//
//
//////////////////////////////////////////
//
//

void Args::_usage(const string &prog_name, ostream &out)
{

  out << endl;

  out << "Program " << prog_name << ":" << endl;
  
  out << "Looks at a directory structure under a specified top\n"
      << "directory and then communicates what it sees to the data\n"
      << "mapper.\n\n";
  
  out <<  "Usage: " << prog_name << " [args as below]\n"
      << "options:\n"
      << "\t[ -h ] produce this list.\n"
      << "\t[ -comp_ext ?] Specify compressed extension (default .gz)\n"
      << "\t[ -debug ] print debug messages.\n"
      << "\t[ -delay ] Specify delay between passes, seconds (default 900)\n"
      << "\t[ -dirsleep ] Seconds to sleep between directory scans.\n"
      << "\t[ -instance ] Specify ProcMap instance. Default is \"primary\"\n"
      << "\t\tDefault is 10 seconds. If set to 0, some debugging\n"
      << "\t\tmessages are printed.\n"
      << "\t[ -once ] run the scout once only.\n"
      << "\t[ -nolock ] do not require a lock file.\n"
      << "\t[ -params ] Specify parameter file. \n"
      << "\t[ -report ] Write _Scout_Report files in processed directories.\n"
      << "\t[ -verbose ] print verbose debug messages.\n"
      << "\n";

  Params::usage(out);

}







