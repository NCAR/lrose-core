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
// SpectraPrint.cc
//
// SpectraPrint object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// SpectraPrint reads IQ time-series data, computes the
// spectra and writes them out to files and SPDB
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <toolsa/pmu.h>
#include <Spdb/DsSpdb.hh>
#include "SpectraPrint.hh"
#include "SpectraPrint.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

SpectraPrint::SpectraPrint(int argc, char **argv)

{

  _beamMgr = NULL;
  _beamReader = NULL;
  constructorOK = true;

  // set programe name

  _progName = "SpectraPrint";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    constructorOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    constructorOK = false;
    return;
  }
  
  // create the beam manager

  _beamMgr = new BeamMgr(_progName, _params);
  if (!_beamMgr->isOK ||
      _beamMgr->readCalFromFile(_params.cal_file_path)) {
    constructorOK = false;
  }
  
  // create the beam reader
  
  _beamReader = new BeamReader(_progName, _params, _args, *_beamMgr);
  if (!_beamReader->constructorOK) {
    constructorOK = false;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

//////////////////////////////////////////////////////////////////
// destructor

SpectraPrint::~SpectraPrint()

{

  if (_beamReader) {
    delete _beamReader;
  }
  
  if (_beamMgr) {
    delete _beamMgr;
  }
  
  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpectraPrint::Run ()
{

  PMU_auto_register("Run");

  while (true) {

    // get next incoming beam

    Beam *beam = _beamReader->getNextBeam();
    if (beam == NULL) {
      return 0;
    }
    
    // load data into print manager
    
    beam->loadManager(*_beamMgr);
  
    // print spectra from current beam
    
    if (_params.write_ascii_files) {
      _beamMgr->printToAscii();
    }

    if (_params.write_to_spdb) {
      _beamMgr->writeToSpdb();
    }

    if (_params.write_to_fmq) {
      _beamMgr->writeToFmq();
    }

    // free up
    
    delete beam;
    
  } // while
  
  return -1;

}

