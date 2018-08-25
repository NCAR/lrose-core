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
// MM5Split.cc
//
// MM5Split object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <mm5/MM5Data.hh>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#include "MM5Split.hh"
#include "SplitV2.hh"
#include "SplitV3.hh"
using namespace std;

// Constructor

MM5Split::MM5Split(int argc, char **argv)

{

  OK = TRUE;
  _input = NULL;

  // set programe name
  
  _progName = "MM5Split";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			    _args.override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // set up input object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.latest_data_info_avail);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug,
			     _args.inputFileList);
  }

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

}

// destructor

MM5Split::~MM5Split()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MM5Split::Run ()
{

  int iret = 0;
  PMU_auto_register("MM5Split::Run");

  char *input_path = NULL;
  while((input_path = _input->next()) != NULL) {

    int version;
    if (MM5Data::getVersion(input_path, version)) {
      iret = -1;
      continue;
    }
    if (_params.debug) {
      cerr << "  File is mm5 output version: " << version << endl;
    }
    
    // if requested, find and print out fortran record headers
    
    if (_params.find_fortran_records) {
      if (_findFortRecs(input_path)) {
	iret = -1;
      }
      continue;
    }
    
    // create and use split objects
    
    if (version == 2) {
      SplitV2 split(_progName, _params, input_path);
      if (split.doSplit()) {
	iret = -1;
      }
    } else {
      SplitV3 split(_progName, _params, input_path);
      if (split.doSplit()) {
	iret = -1;
      }
    }

  } // while

  return iret;

}

//////////////////////////////////
// Find the FORTRAN record headers
//
// Returns 0 on success, -1 on failure

int MM5Split::_findFortRecs(const char *input_path)
  
{
  
  // open the file

  FILE *in;
  if ((in = fopen(input_path, "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - MM5Split::_findFortRecs" << endl;
    cerr << " Cannot open input file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  while (!feof(in)) {
    long start_reclen = _readFortRecLen(in);
    if (start_reclen < 0) {
      break;
    }
    cout << "Start F rec len: " << start_reclen << endl;
    fseek(in, start_reclen, SEEK_CUR);
    long end_reclen = _readFortRecLen(in);
    if (end_reclen < 0) {
      break;
    }
    cout << "End F rec len: " << end_reclen << endl;
    cout << "=============================" << endl;
    if (start_reclen != end_reclen) {
      cerr << "ERROR - start and end reclen not the same" << endl;
      fclose(in);
      return -1;
    }
  }
  
  fclose(in);
  return 0;

}

///////////////////////////////
// _readFortRecLen()
//
// Read a fortran record length
//

long MM5Split::_readFortRecLen(FILE *in)
  
{
  
  si32 reclen;
  
  if (ufread(&reclen, sizeof(si32), 1, in) != 1) {
    if (feof(in)) {
      return -1;
    }
    int errNum = errno;
    cerr << "ERROR - SplitV3::_readFortRecLen" << endl;
    cerr << "Cannot read fortran rec len" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  BE_to_array_32(&reclen, sizeof(si32));

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "    Fortran rec len: %d\n", reclen);
  }

  return reclen;

}

///////////////////////////////
// copyBuffer()
//
// copy a buffer from one file to another
//
// Returns 0 on success, -1 on failure

int MM5Split::copyBuffer(FILE *in, FILE *out,
			 long in_offset,
			 long nbytes)
  
{
  
  int bufsize = BUFSIZ;
  ui08 buf[BUFSIZ];
  
  // copy over the data set
  
  fseek(in, in_offset, SEEK_SET);
  long nleft = nbytes;
  while (nleft > 0) {
    if (nleft < BUFSIZ) {
      bufsize = nleft;
    }
    if (ufread(buf, 1, bufsize, in) != bufsize) {
      int errNum = errno;
      cerr << "ERROR - MM5Split::copyBuffer" << endl;
      cerr << "  Cannot read for buffer copy" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if (ufwrite(buf, 1, bufsize, out) != bufsize) {
      int errNum = errno;
      cerr << "ERROR - MM5Split::copyBuffer" << endl;
      cerr << "  Cannot write to buffer" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    nleft -= bufsize;
  }
    
  return 0;

}
