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
// July 1999
//
//////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <cstring>
#include <iostream>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
using namespace std;

// parse

int Args::parse(int argc, char **argv, string &prog_name)

{

  int iret = 0;
  char tmp_str[256];
  vector<string> fields;

  // intialize

  TDRP_init_override(&override);
  startTime = 0;
  endTime = 0;

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
      
    } else if (!strcmp(argv[i], "-i")) {
      if (i < argc - 1) {
        sprintf(tmp_str, "instance = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-mode")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "mode = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-in_url")) {

      if(i < argc - 1) {
        sprintf(tmp_str, "input_url = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-out_url")) {

      if(i < argc - 1) {
        sprintf(tmp_str, "output_url = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      }
      else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-out_path")) {

      if(i < argc - 1) {
        sprintf(tmp_str, "output_path = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        sprintf(tmp_str, "write_to_path = TRUE;");
        TDRP_add_override(&override, tmp_str);
      }
      else {
        iret = -1;
      }

    } else if (!strcmp(argv[i], "-writeLdataInfo")) {
      
      if (i < argc - 1) {
        sprintf(tmp_str, "writeLdataInfo = %s;", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
        i=i+1; if (i>argc) continue;
      } else {
        iret = -1;
      }
      
    } else if (!strcmp(argv[i], "-start")) {
      
      if (i < argc - 1) {
        startTime = DateTime::parseDateTime(argv[++i]);
        if (startTime == DateTime::NEVER)
        {
          iret = -1;
        }
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-end")) {
      
      if (i < argc - 1) {
        endTime = DateTime::parseDateTime(argv[++i]);
        if (endTime == DateTime::NEVER)
        {
          iret = -1;
        }
      } else {
        iret = -1;
      }
        
    } else if (!strcmp(argv[i], "-xml")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_XML;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-mdv")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_MDV;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-netcdf3")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NCF;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "ncf_file_format = CLASSIC;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "ncf_compress_data = FALSE;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-netcdf4")) {
      
      sprintf(tmp_str, "output_format = OUTPUT_FORMAT_NCF;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "ncf_file_format = NETCDF4;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "ncf_compress_data = TRUE;");
      TDRP_add_override(&override, tmp_str);
      
      sprintf(tmp_str, "ncf_compression_level = 4;");
      TDRP_add_override(&override, tmp_str);
      
    } else if (!strcmp(argv[i], "-field")) {
      
      if (i < argc - 1) {
	fields.push_back(argv[++i]);
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-if") ||
               !strcmp(argv[i], "-f")) {
        
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

  // set fields if specified

  if (fields.size() > 0) {
    
    sprintf(tmp_str, "set_field_names = true;");
    TDRP_add_override(&override, tmp_str);
    
    string fieldsStr = "field_names = { ";
    for (size_t ii = 0; ii < fields.size(); ii++) {
      fieldsStr += "\"";
      fieldsStr += fields[ii];
      fieldsStr += "\"";
      if (ii != fields.size() - 1) {
        fieldsStr += ", ";
      } else {
        fieldsStr += " ";
      }
    }
    fieldsStr += "};";
    TDRP_add_override(&override, fieldsStr.c_str());
    
  } // if (fields.size() ...

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
      << "       [ -d, -debug ] print debug messages\n"
      << "       [ -end \"yyyy mm dd hh mm ss\"] end time\n"
      << "       [ -in_url url] Input URL\n"
      << "       [ -mdv ] write out as MDV\n"
      << "       [ -netcdf3 ] write out as netcdf3\n"
      << "       [ -netcdf4 ] write out as netcdf4\n"
      << "       [ -32bit ] write using 32-bit headers\n"
      << "       [ -out_url url] Output URL\n"
      << "       [ -out_path path] Output path\n"
      << "         Writes to specified path instead of URL\n"
      << "       [ -writeLdataInfo ?] Write LdataInfo files for output data\n"
      << "       [ -if ?, -f ?] input file list\n"
      << "         Sets mode to FILELIST\n"
      << "       [ -mode ?] ARCHIVE, ARCHIVE_FCST, REALTIME  or FILELIST\n"
      << "       [ -start \"yyyy mm dd hh mm ss\"] start time\n"
      << "       [ -v, -verbose ] print verbose debug messages\n"
      << "       [ -xml ] write out as xml\n"
      << endl;

  out << "Note: you must specify start and end dates." << endl << endl;
  
  Params::usage(out);

}
