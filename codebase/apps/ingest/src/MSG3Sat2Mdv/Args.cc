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
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args (const string &prog_name)
 
{
  _progName = prog_name;
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (const int argc, char **argv)

{

  if (argc == 1)
    {
      _usage(cerr);
      return 1;
    }
    
  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;
  bool got_start = false;
  bool got_end = false;


  TDRP_init_override(&override);

  // loop through args
  
  for (int i =  1; i < argc; i++) 
    {
      
      if (!strcmp(argv[i], "--") ||
	  !strcmp(argv[i], "-h") ||
	  !strcmp(argv[i], "-help") ||
	  !strcmp(argv[i], "-man")) 
	{
	  
	  _usage(cout);
	  exit (0);
	  
	} 
      else if (!strcmp(argv[i], "-debug")) 
	{
	  
	  sprintf(tmp_str, "debug = DEBUG_NORM;");
	  TDRP_add_override(&override, tmp_str);
	  
	} 
      else if (!strcmp(argv[i], "-verbose")) 
	{
	  
	  sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
	  TDRP_add_override(&override, tmp_str);
	  
	} 
      else if (!strcmp(argv[i], "-mode")) 
	{
	  
	  if (i < argc - 1) 
	    {
	      sprintf(tmp_str, "mode = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    } 
	  else 
	    {
	      iret = -1;
	    }	  
	} 
      else if (!strcmp(argv[i], "-indir")) 
	{
	  
	  if (i < argc - 1) 
	    {
	      sprintf(tmp_str, "input_dir = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    } 
	  else 
	    {
	      iret = -1;
	    }	  
	} 
      else if (!strcmp(argv[i], "-outurl")) 
	{
	  
	  if (i < argc - 1) 
	    {
	      sprintf(tmp_str, "output_url = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    } 
	  else 
	    {
	      iret = -1;
	    }	  
	} 
      else if (!strcmp(argv[i], "-start")) 
	{
	  if (i < argc - 1){
	    got_start = true;
	    sprintf(tmp_str, "start_time = %s;", argv[++i]);
	    TDRP_add_override(&override, tmp_str);
	  } 
	  else 
	    {
	      cerr << "Need start time\n";
	      iret = -1;
	    }
	} 
      else if (!strcmp(argv[i], "-end")) 
	{
	  if (i < argc -1 )
	    {
	      got_end = true;
	      sprintf(tmp_str, "end_time = %s;", argv[++i]);
	      TDRP_add_override(&override, tmp_str);
	    } 
	  else 
	    {
	      cerr << "Need end time\n";
	      iret = -1;
	    }
	}

      // file list specification

      else if ( !strcmp(argv[i], "-f" ) || !strcmp(argv[i], "-file" ))
	{

	  // search for next arg which starts with '-'

	  int j;
	  
	  for( j = i+1; j < argc; j++ ) 
	    {
	      if (argv[j][0] == '-')
		break;
	      else
		inputFileList.push_back( argv[j] );
	    }

	  // Increment i to skip over the files

	  i = i + (int)inputFileList.size();

	  TDRP_add_override( &override, "mode = FILELIST;" );

	  if ( inputFileList.size() == 0 ) 
	    {
	      cerr << "Missing file list specification.";
	      iret = -1;
	    }
	}
      
      else if ( !strcmp(argv[i], "-params" )) 
	{

	  // skip over the param file

	  i++;
	}
      else if ( !strcmp(argv[i], "-print_params" )) 
	{

	  // do nothing 

	  
	}
      else 
	{
	  cerr << "WARNING - unknown command line arg: " << argv[i] << endl;
	}
    }

  // If start and end times specified, go to TIME_INTERVAL mode.

  if ( (got_start && !got_end) || (!got_start && got_end))
    {
      cerr << "Need both start and end times for TIME_INTERVAL mode\n";
      
      iret = -1;
    }
      
  if ((got_start) && (got_end))
    {
      TDRP_add_override(&override, "mode = TIME_INTERVAL;");
    }
  
  

  if (iret) 
    {
      _usage(cerr);
    }
  
  return (iret);
    
}

void Args::_usage(ostream &out)

{

  out << "Usage: " << _progName << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -end \"yyyymmddhhmmss\"] end time\n"
      << "          TIME_INTERVAL mode only\n"
      << "       [ -f, -file ???] specify list of files to convert\n"
      << "       [ -indir ?] specify input dir\n"
      << "       [ -mode ?] REALTIME, TIME_INTERVAL, or FILELIST \n"
      << "       [ -outurl ?] specify output URL for MDV files\n"
      << "       [ -start \"yyyymmddhhmmss\"] start time\n"
      << "          TIME_INTERVAL mode only\n"
      << "       [ -verbose ] print verbose debug messages\n"
      << endl;

  out << "NOTE: For TIME_INTERVAL mode, you must specify the \n"
      << "analysis times using -start and -end.\n"
      << endl;

  out << "For FILE_LIST mode, you must specify a list of files with "
      << "the -file or -f options\n" << endl;

  Params::usage(out);

}
