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
#include "Args.hh"
#include <toolsa/umisc.h>
#include <ctime>

using std::string;
using std::cout;
using std::cerr;
using std::ostream;
using std::endl;


// constructor

Args::Args()

{
  _path = "";
  _suffix = "";
  _dataType = "mdv";
  _fileExt = "mdv";
  _isFcast = false;
  _debug = false;
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
      
    } else if (!strcmp(argv[i], "-path")) {
      
      if (i < argc - 1) {
	_path = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-suffix")) {
      
      if (i < argc - 1) {
	_suffix = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-dtype")) {
      
      if (i < argc - 1) {
	_dataType = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ext")) {
      
      if (i < argc - 1) {
	_fileExt = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-forecast")) {

      _isFcast = true;

    } else if (!strcmp(argv[i], "-debug")) {
      
      _debug = true;
    }
  }      

  if (_path.empty() || _suffix.empty())
  {
    cerr << "Need to set path and suffix" << endl;
    iret = -1;
  }

  if (iret) {
    _usage(prog_name, cerr);
  }

  return (iret);
    
}

void Args::_usage(string &prog_name, ostream &out)
{

  out << prog_name << " allows you to write a latest_data_info file\n"
      << "  from a path, by parsing needed info from the path\n"
      << endl;

  out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -h, -help, -man ] produce this list.\n"
      << "       [ -debug ] print debug messages\n"
      << "       [ -path ? ] full path (required argument)\n"
      << "                   path is relative to $RAP_DATA_DIR unless it\n"
      << "                   starts with . or /\n"
      << "       [ -suffix ? ] File suffix, required argument\n"
      << "       [ -dtype ? ] data type - for DataMapper (mdv, nc,...),\n"
      << "                    default is mdv\n"
      << "       [ -ext ? ] file extension (mdv, nc,...) default is mdv\n"
      << "       [ -info1 ? ] user info 1 (optional)\n"
      << "       [ -info2 ? ] user info 2 (optional)\n"
      << "       [ -forecast] Set this for forecast data\n"
      << "\n"
      << "  Typical use:\n"
      << "   " << prog_name 
      << " -path <lPath>/yyyymmdd/g_hhmmss/f_dddddddd.<s> -suffix <s> -forecast\n"
      << "\n"
      << "   " << prog_name 
      << " -path <lPath>/yyyymmdd/hhmmss.<s> -suffix <s>\n"
      << "\n"
      << endl;
}



















