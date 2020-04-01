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
// TestRadxMsg.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// TestRadxMsg test the serialization and deserialization of Radx
// objects, using RadxMsg.
//
////////////////////////////////////////////////////////////////

#include <vector>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/toolsa_macros.h>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxMsg.hh>
#include "TestRadxMsg.hh"
using namespace std;

// Constructor

TestRadxMsg::TestRadxMsg(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name
  
  _progName = "TestRadxMsg";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // create the data input object

  if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  return;

}

// destructor

TestRadxMsg::~TestRadxMsg()

{

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int TestRadxMsg::Run()
{
  
  int iret = 0;

  // loop until end of data
  
  _input->reset();
  char *input_file_path;
  
  while ((input_file_path = _input->next()) != NULL) {
    
    if (_processFile(input_file_path)) {
      cerr << "ERROR - TestRadxMsg::Run" << endl;
      cerr << "  Cannot process file: " << input_file_path << endl;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// process this file

int TestRadxMsg::_processFile(const string filePath)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  // set up RadxFile object
  
  GenericRadxFile file;
  _setupRead(file);
  
  // read in file

  if (file.readFromPath(filePath, _vol)) {
    cerr << "ERROR - TestRadxMsg::Run" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  _readPaths = file.getReadPaths();
  if (_params.debug) {
    cerr << "Following file paths used: " << endl;
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "    " << _readPaths[ii] << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _vol.printWithRayMetaData(cerr);
    // _vol.printWithFieldData(cerr);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _vol.print(cerr);
  }

  if (_params.test_type == Params::TEST_RADX_FIELDS) {
    return _testRadxFields();
  } else if (_params.test_type == Params::TEST_RADX_RAYS) {
    return _testRadxRays();
  } else if (_params.test_type == Params::TEST_RADX_VOL) {
    return _testRadxVol();
  }
  
  return 0;

}

//////////////////////////////////////////////////
// set up read 

void TestRadxMsg::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.read_set_field_names) {
    for (int i = 0; i < _params.read_field_names_n; i++) {
      file.addReadField(_params._read_field_names[i]);
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

//////////////////////////////////////////////////
// test radx fields from the volume

int TestRadxMsg::_testRadxFields()
{

  // check file for non-zero rays

  if (_vol.getNRays() == 0) {
    cerr << "ERROR - TestRadxMsg::_testRadxFields()" << endl;
    cerr << "  No rays in volume" << endl;
    cerr << "  File: " << _vol.getPathInUse() << endl;
    return -1;
  }
  
  // open output files

  if (ta_makedir_for_file(_params.text_path_before)) {
    cerr << "ERROR - TestRadxMsg::_testRadxFields()" << endl;
    cerr << "  Cannot make dir for before file: " 
         << _params.text_path_before << endl;
    return -1;
  }
  
  if (ta_makedir_for_file(_params.text_path_after)) {
    cerr << "ERROR - TestRadxMsg::_testRadxFields()" << endl;
    cerr << "  Cannot make dir for after file: " 
         << _params.text_path_after << endl;
    return -1;
  }

  ofstream beforeFile(_params.text_path_before);
  ofstream afterFile(_params.text_path_after);

  if (_params.debug) {
    cerr << "Writing 'before' fields to file: " << _params.text_path_before << endl;
  }
  if (_params.debug) {
    cerr << "Writing 'after' fields to file: " << _params.text_path_after << endl;
  }
  
  // loop through the rays
  
  const vector<RadxRay *> &rays = _vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    vector<RadxField *> flds = ray->getFields();
    for (size_t ifield = 0; ifield < flds.size(); ifield++) {

      RadxField *field = flds[ifield];
  
      // print the contents of the before field for comparison
      
      field->printWithData(beforeFile);
      
      // serialize the field into a RadxMsg
      
      RadxMsg msg(RadxMsg::RadxFieldMsg);
      field->serialize(msg);
      
      // deserialize the field from the RadxMsg

      RadxField copy;
      if (copy.deserialize(msg)) {
        cerr << "ERROR - TestRadxMsg::_testRadxFields()" << endl;
        cerr << "  Cannot deserialize field from msg" << endl;
        msg.printHeader(cerr, "  ");
        return -1;
      }

      // print the contents of the after field for comparison
      
      copy.printWithData(afterFile);

    } // ifield

  } // iray

  // close files

  beforeFile.close();
  afterFile.close();

  return 0;

}

//////////////////////////////////////////////////
// test radx rays from volume

int TestRadxMsg::_testRadxRays()
{

  // check file for non-zero rays

  if (_vol.getNRays() == 0) {
    cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
    cerr << "  No rays in volume" << endl;
    cerr << "  File: " << _vol.getPathInUse() << endl;
    return -1;
  }
  
  // open output files

  if (ta_makedir_for_file(_params.text_path_before)) {
    cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
    cerr << "  Cannot make dir for before file: " 
         << _params.text_path_before << endl;
    return -1;
  }
  
  if (ta_makedir_for_file(_params.text_path_after)) {
    cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
    cerr << "  Cannot make dir for after file: " 
         << _params.text_path_after << endl;
    return -1;
  }

  ofstream beforeFile(_params.text_path_before);
  ofstream afterFile(_params.text_path_after);

  if (_params.debug) {
    cerr << "Writing 'before' fields to file: " << _params.text_path_before << endl;
  }
  if (_params.debug) {
    cerr << "Writing 'after' fields to file: " << _params.text_path_after << endl;
  }
  
  // loop through the rays
  
  const vector<RadxRay *> &rays = _vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    
    // print the contents of the before ray for comparison
    
    ray->printWithFieldData(beforeFile);
      
    // serialize the ray into a RadxMsg
      
    RadxMsg msg(RadxMsg::RadxRayMsg);
    ray->serialize(msg);
    
    // deserialize the field from the RadxMsg
    
    RadxRay copy;
    if (copy.deserialize(msg)) {
      cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
      cerr << "  Cannot deserialize ray from msg" << endl;
      msg.printHeader(cerr, "  ");
      return -1;
    }
    
    // print the contents of the after field for comparison
    
    copy.printWithFieldData(afterFile);
    
  } // iray

  // close files

  beforeFile.close();
  afterFile.close();

  return 0;

}

//////////////////////////////////////////////////
// test a radx vol

int TestRadxMsg::_testRadxVol()
{

  // open output files

  if (ta_makedir_for_file(_params.text_path_before)) {
    cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
    cerr << "  Cannot make dir for before file: " 
         << _params.text_path_before << endl;
    return -1;
  }
  
  if (ta_makedir_for_file(_params.text_path_after)) {
    cerr << "ERROR - TestRadxMsg::_testRadxRays()" << endl;
    cerr << "  Cannot make dir for after file: " 
         << _params.text_path_after << endl;
    return -1;
  }

  ofstream beforeFile(_params.text_path_before);
  ofstream afterFile(_params.text_path_after);

  if (_params.debug) {
    cerr << "Writing 'before' volume to text file: "
         << _params.text_path_before << " ....." << endl;
  }
  
  // print the contents of the before vol for comparison
  
  _vol.printWithFieldData(beforeFile);
  
  // serialize the vol into a RadxMsg
  
  if (_params.debug) {
    cerr << "Serializing volume into msg ..... " << endl;
  }
  
  RadxMsg msg(RadxMsg::RadxVolMsg);
  _vol.serialize(msg);
    
  // deserialize from the RadxMsg
    
  if (_params.debug) {
    cerr << "Deserializing volume from msg ..... " << endl;
  }
  
  RadxVol copy;
  if (copy.deserialize(msg)) {
    cerr << "ERROR - TestRadxMsg::_testRadxVol()" << endl;
    cerr << "  Cannot deserialize vol from msg" << endl;
    msg.printHeader(cerr, "  ");
    return -1;
  }
    
  // print the contents of the after field for comparison
  
  if (_params.debug) {
    cerr << "Writing 'after' volume to text file: "
         << _params.text_path_after << " ....." << endl;
  }

  copy.printWithFieldData(afterFile);
    
  return 0;

}


