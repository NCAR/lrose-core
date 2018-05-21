/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// CwbRadar2Mdv.cc
//
// CwbRadar2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#include "CwbRadar2Mdv.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <didss/DsInputPath.hh>
#include <dataport/swap.h>
#include <cerrno>
#include <cstdlib>
using namespace std;

// Constructor

CwbRadar2Mdv::CwbRadar2Mdv(int argc, char **argv)

{

  OK = TRUE;
  _input = NULL;

  // set programe name

  _progName = "CwbRadar2Mdv";
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

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(), _params.instance,
		  PROCMAP_REGISTER_INTERVAL);
    PMU_auto_register("In CwbRadar2Mdv constructor");
  }

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    bool use_ldata_info = _params.use_latest_data_info_file;
    bool latest_file_only = true;
    _input = new DsInputPath((char *) _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.realtime_input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     use_ldata_info,
			     latest_file_only);
  } else {
    // ARCHIVE mode
    _input = new DsInputPath((char *) _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.filePaths);
  }

  return;

}

// destructor

CwbRadar2Mdv::~CwbRadar2Mdv()

{

  // free up

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int CwbRadar2Mdv::Run ()
{

  int iret = 0;
  
  PMU_auto_register("CwbRadar2Mdv::Run");
  
  // loop through available files
  
  char *filePath;
  while ((filePath = _input->next()) != NULL) {

    if (_processFile(filePath)) {
      iret = -1;
    }
    
  }
  
  return iret;
  
}

//////////////////////////////////////////////////
// Process the file

int CwbRadar2Mdv::_processFile (const char *file_path)

{

  if (_params.debug) {
    cerr << "  Processing file: " << file_path << endl;
  }

  InputFile inFile(_progName, _params);

  if (inFile.readFile(file_path)) {
    cerr << "ERROR - CwbRadar2Mdv::_processFile" << endl;
    cerr << "  Cannot read input file: " << file_path << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }

  if (_params.debug) {
    inFile.printInfo(cerr);
    // inFile.printHeader(cerr);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.printData(cerr);
  }

  // create output object, load up with data

  OutputFile outFile(_progName, _params);
  outFile.loadData(inFile);

  // write the file
  
  if (outFile.writeVol()) {
    cerr << "ERROR - CwbRadar2Mdv::_processFile" << endl;
    cerr << "  Cannot write output volume." << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }

  return 0;

}

