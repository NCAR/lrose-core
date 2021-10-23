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
////////////////////////////////////////////////////////////////////////
// FmqTest.cc
//
// FmqTest object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2021
//
///////////////////////////////////////////////////////////////////////
//
// FmqTest reads data from an input file,
// and writes the contents to an FMQ at a specified rate.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/TaFile.hh>
#include "FmqTest.hh"
using namespace std;

// Constructor

FmqTest::FmqTest(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "FmqTest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // allocate output FMQs

  _prevTimeForOpen = 0;

  // logging

  _msgLog = new MsgLog(_progName);

  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

FmqTest::~FmqTest()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FmqTest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // open the input file

  TaFile inFile;
  FILE *in = inFile.fopen(_params.input_file_path, "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - FmqTest::Run()" << endl;
    cerr << "  Cannot open file path: " << _params.input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // stat the file to get the size

  if (inFile.fstat()) {
    int errNum = errno;
    cerr << "ERROR - FmqTest::Run()" << endl;
    cerr << "  Cannot stat file path: " << _params.input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  struct stat &fstat = inFile.getStat();
  
  // read contents into buffer

  vector<char> fileContents;
  fileContents.resize(fstat.st_size);
  if (inFile.fread(fileContents.data(), 1, fstat.st_size) != (size_t) fstat.st_size) {
    int errNum = errno;
    cerr << "ERROR - FmqTest::Run()" << endl;
    cerr << "  Cannot read in file path: " << _params.input_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  inFile.fclose();

  // open the output fmq

  DsFmq fmq;
  if (fmq.initReadWrite(_params.output_fmq_url,
                        _progName.c_str(),
                        (_params.debug >= Params::DEBUG_VERBOSE),
                        DsFmq::END,
                        _params.output_compress,
                        _params.output_n_slots,
                        _params.output_buf_size,
                        -1, _msgLog)) {
    cerr << "ERROR - FmqTest::Run" << endl;
    cerr << "  Cannot open output FMQ at url: " << _params.output_fmq_url << endl;
    return -1;
  }
  if (_params.data_mapper_reg_interval > 0) {
    fmq.setRegisterWithDmap(true, _params.data_mapper_reg_interval);
  }
  if (_params.write_blocking) {
    fmq.setBlockingWrite();
    fmq.setSingleWriter();
  }
    
  if (_params.debug) {
    cerr << "Opened output fmq to URL: " << _params.output_fmq_url << endl;
  }
    
  // write to the fmq

  int count = 0;
  
  while (true) {
    
    PMU_auto_register("write to output");
    if (fmq.writeMsg(0, 0, fileContents.data(), fileContents.size())) {
      cerr << "ERROR - FmqTest::Run" << endl;
      cerr << "  Writing message to url: " << _params.output_fmq_url << endl;
      cerr << "  Message len: " << fileContents.size() << endl;
      cerr << "  " << fmq.getErrStr() << endl;
      fmq.closeMsgQueue();
      return -1;
    }
    count++;

    if (_params.debug) {
      cerr << "Success writing message to fmq: " << _params.output_fmq_url << endl;
      cerr << "Total count: " << count << endl;
    }

    if (_params.write_count > 0 && count >= _params.write_count) {
      break;
    }

    umsleep(_params.write_sleep_msecs);

  } // while

  fmq.closeMsgQueue();
  return 0;

}

