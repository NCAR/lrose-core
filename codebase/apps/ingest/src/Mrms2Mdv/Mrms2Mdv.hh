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
/////////////////////////////////////////////////////////////////////
// Mrms2Mdv top-level application class header.
//
// Converts MRMS (Multi-Radar Multi-Sensor) data from binary to Mdv.
//
// -Jason Craig-  Aug 2013
////////////////////////////////////////////////////////////////////

#ifndef MRMS_TO_MDV
#define MRMS_TO_MDV

#include <string>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/MemBuf.hh>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include "Params.hh"
#include "Args.hh"
#include "OutputFile.hh"

using namespace std;

class Mrms2Mdv {
 public:

  // instance -- create the Singleton
  static Mrms2Mdv *Inst(int argc, char **argv);
  static Mrms2Mdv *Inst();
  
  ~Mrms2Mdv();
  
  //
  // Initialization
  //
  int init( int argc, char**argv );
    
  // Flag indicating whether the program status is currently okay.
  bool okay;
  
  //
  // Execution
  //
  int run();
   
 private:
  
  // 
  // Initialization
  //
  Path _program;
  
  // Constructor -- private because this is a singleton object
  Mrms2Mdv(int argc, char **argv);

  void _usage();
  int _processArgs( int argc, char **argv,
		    tdrp_override_t& override ,
		    int* nFiles, char*** fileList );
  bool _gotField(char* name);
  int _getData();
  bool _gotRequiredFields();
  int _writeMdvFile(time_t generateTime, int debug);
  void _clearMdvxFields();

   
  //
  // Singleton instance pointer
  //
  static Mrms2Mdv *_instance;  // singleton instance

  //
  // Parameter processing
  //
  char   *_paramsPath;
  int _processParams( int nFiles, char** fileList );
  
  // Program parameters.
  Args *_args;
  Params *_params;
  DsInputPath *_inputPath;
  OutputFile *_outputFile;

  Mdvx::field_header_t _fieldHeader;
  Mdvx::vlevel_header_t _vlevelHeader;
  char *_progName;
  int _nfiles;
  long _curTime;
  char *_flist;
  vector<MdvxField*> _outputFields;
  MemBuf *_data;
  void *_dataPtr;

  const static int nFields;
  const static char *fields[20];
  bool *_found;

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Mrms2Mdv");
  }


};

//
// Defines for success and failure returns
//
#define RI_FAILURE -1
#define RI_SUCCESS 0

#endif



