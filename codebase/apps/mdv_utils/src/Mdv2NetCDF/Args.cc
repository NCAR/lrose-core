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
// Aug 2001
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <string.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Args::Args ()

{
  startTime = 0;
  endTime = 0;
  TDRP_init_override(&override);
  additional_tdrp_file = NULL;
}


// Destructor

Args::~Args ()

{
  TDRP_free_override(&override);
}

// Parse command line
// Returns 0 on success, -1 on failure

int Args::parse (int argc, char **argv,
                 const string &prog_name)

{

  char tmp_str[BUFSIZ];

  // intialize

  int iret = 0;

  // loop through args

  for (int i =  1; i < argc; i++) {
    if (!strcmp(argv[i], "-additional_tdrp_file")) {
      i++;
      if (i >= argc){
        fprintf(stderr,"*** Error: -additional_tdrp_file needs an argument.\n");
        exit(-1);
      }
      additional_tdrp_file = argv[i];

    } else if (!strcmp(argv[i], "--") ||
        !strcmp(argv[i], "-h") ||
        !strcmp(argv[i], "-help") ||
        !strcmp(argv[i], "-man")) {

      _usage(cout, prog_name);
      exit (0);

    } else if (!strcmp(argv[i], "-debug") ||
               !strcmp(argv[i], "-d")) {
      
      sprintf(tmp_str, "debug = DEBUG_NORM;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-verbose") ||
               !strcmp(argv[i], "-v")) {

      sprintf(tmp_str, "debug = DEBUG_VERBOSE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-forecast")) {
      
      sprintf(tmp_str, "output_as_forecast = TRUE;");
      TDRP_add_override(&override, tmp_str);

    } else if (!strcmp(argv[i], "-noforecast")) {
      
      sprintf(tmp_str, "output_as_forecast = FALSE;");
      TDRP_add_override(&override, tmp_str);

    } else if(!strcmp(argv[i], "-i")) {
      if (i < argc - 1) {
        sprintf(tmp_str, "instance = \"%s\";", argv[++i]);
	TDRP_add_override(&override, tmp_str);
      }
      else {
	iret = -1;        
      }
    } else if (!strcmp(argv[i], "-mode")) {

      if (i < argc - 1) {
        sprintf(tmp_str, "mode = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-outdir")) {
	  
      if (i < argc - 1) {
        sprintf(tmp_str, "output_dir = \"%s\";", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }	  

    } else if (!strcmp(argv[i], "-start")) {

      if (i < argc - 1) {
        date_time_t start;
        i++;
        if( (sscanf(argv[i], "%d %d %d %d %d %d",
                   &start.year, &start.month, &start.day,
                   &start.hour, &start.min, &start.sec) != 6)
            &&
            (sscanf(argv[i], "%d_%d_%d_%d_%d_%d",
                   &start.year, &start.month, &start.day,
                   &start.hour, &start.min, &start.sec) != 6)
          )
        {
          iret = -1;
        } else {
          uconvert_to_utime(&start);
          startTime = start.unix_time;
          sprintf(tmp_str, "mode = TIME_INTERVAL;");
          TDRP_add_override(&override, tmp_str);
        }
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-end")) {

      if (i < argc - 1) {
        date_time_t end;
        i++;
        if( (sscanf(argv[i], "%d %d %d %d %d %d",
                   &end.year, &end.month, &end.day,
                   &end.hour, &end.min, &end.sec) != 6)
            &&
            (sscanf(argv[i], "%d_%d_%d_%d_%d_%d",
                   &end.year, &end.month, &end.day,
                   &end.hour, &end.min, &end.sec) != 6)
          )
        {
          iret = -1;
        } else {
          uconvert_to_utime(&end);
          endTime = end.unix_time;
          sprintf(tmp_str, "mode = TIME_INTERVAL;");
          TDRP_add_override(&override, tmp_str);
        }
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-f")) {

      sprintf(tmp_str, "mode = FILELIST;");
      TDRP_add_override(&override, tmp_str);

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

    } else if (!strcmp(argv[i], "-o")) {
      sprintf(tmp_str, "use_output_filename = true;");
      TDRP_add_override(&override, tmp_str);

      if ((i+1) < argc)
        {
          sprintf(tmp_str, "output_filename = \"%s\";", argv[i+1] );
          cerr << " here is the tmp_str: " << tmp_str << endl;
          TDRP_add_override(&override, tmp_str);
        }
      else
        iret = 1;

    } else if (!strcmp(argv[i], "-prefix")) {
      if ((i+1) < argc)
        {
          sprintf(tmp_str, "output_file_prefix = \"%s\";", argv[i+1] );
          cerr << " here is the tmp_str: " << tmp_str << endl;
          TDRP_add_override(&override, tmp_str);
        }
      else
      {
        iret = 1;
      }
    }
  } // i

  if (iret) {
    _usage(cerr, prog_name);
  }

  return (iret);

}

void Args::_usage(ostream &out,
                  const string &prog_name)

{

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\" or yyyy_mm_dd_hh_mm_ss] end time\n"
      << "          forces TIME_INTERVAL mode only\n"
      << "       [ -f files ] specify input file list.\n"
      << "         forces FILELIST mode\n"
      << "       [ -forecast ] write out in forecast mode\n"
      << "          i.e. use yyyymmdd/g_hhmmdd/f_llllllll.mdv structure\n"
      << "       [ -prefix file prefix ] specify output file prefix.\n"
      << "       [ -o filename ] specify output filename.\n"
      << "       [ -outdir ?] specify output directory for MDV files\n"
      << "       [ -mode ?] TIME_INTERVAL, REALTIME or FILELIST\n"
      << "       [ -noforecast ] do not write out in forecast mode\n"
      << "                       i.e. use valid time mode\n"
      << "                       use yyyymmdd/hhmmss.mdv structure\n"
      << "       [ -start \"yyyy mm dd hh mm ss\" or yyyy_mm_dd_hh_mm_ss] start time\n"
      << "          forces TIME_INTERVAL mode\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -additional_tdrp_file ] specify a TDRP file whose\n"
      << "          params will override those in the params file.\n"
      << endl;

  Params::usage(out);

}







