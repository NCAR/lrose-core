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
// HawkEye.cc
//
// HawkEye display
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// HawkEye is the engineering display for the Hawk radar system
//
///////////////////////////////////////////////////////////////

#include "HawkEye.hh"
#include "PolarManager.hh"
#include "BscanManager.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"

#include <string>
#include <iostream>
#include <QtWidgets/QApplication>

using namespace std;

// Constructor

HawkEye::HawkEye(int argc, char **argv) :
        _args("HawkEye"),
        _argc(argc),
        _argv(argv)

{

  OK = true;
  _polarManager = NULL;
  _bscanManager = NULL;
  _reader = NULL;

  // set programe name

  _progName = strdup("HawkEye");
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
    return;
  }

  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }

  // check for any filtered fields

  _haveFilteredFields = false;
  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    if (strlen(_params._fields[ifield].filtered_name) > 0) {
      _haveFilteredFields = true;
    }
  }

  // set params on alloc checker

  AllocCheck::inst().setParams(&_params);

  // set up display fields

  if (_setupDisplayFields()) {
    OK = false;
    return;
  }

  // create reader

  if (_setupReader()) {
    OK = false;
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

HawkEye::~HawkEye()

{

  if (_polarManager) {
    delete _polarManager;
  }

  if (_bscanManager) {
    delete _bscanManager;
  }

  if (_reader) {
    delete _reader;
  }

  for (size_t ii = 0; ii < _displayFields.size(); ii++) {
    delete _displayFields[ii];
  }
  _displayFields.clear();

}

//////////////////////////////////////////////////
// Run

int HawkEye::Run(QApplication &app)
{

  // start the reader thread

  _reader->signalRunToStart();
  
  if (_params.display_mode == Params::POLAR_DISPLAY) {
    
    _polarManager = new PolarManager(_params, _reader,
                                     _displayFields, _haveFilteredFields);

    if (_args.inputFileList.size() > 0) {
      _polarManager->setInputFileList(_args.inputFileList);
      // override archive data url from input file
      string url = _getArchiveUrl(_args.inputFileList[0]);
      TDRP_str_replace(&_params.archive_data_url, url.c_str());
    }

    return _polarManager->run(app);

  } else if (_params.display_mode == Params::BSCAN_DISPLAY) {

    _bscanManager = new BscanManager(_params, _reader, 
                                     _displayFields, _haveFilteredFields);
    return _bscanManager->run(app);

  }
  
  return -1;

}

//////////////////////////////////////////////////
// set up reader thread
// returns 0 on success, -1 on failure
  
int HawkEye::_setupReader()
{
  
  switch (_params.input_mode) {
    
    case Params::DSR_FMQ_INPUT:
    case Params::IWRF_FMQ_INPUT:
    case Params::IWRF_TCP_INPUT: {
      IwrfReader *iwrfReader = new IwrfReader(_params);
      _reader = iwrfReader;
      break;
    }
      
    case Params::SIMULATED_RHI_INPUT: {
      
      SimRhiReader *simReader = new SimRhiReader(_params);
      _reader = simReader;
      
      vector<SimRhiReader::Field> simFields;
      for (size_t ii = 0; ii < _displayFields.size(); ii++) {
        SimRhiReader::Field simField;
        simField.name = _displayFields[ii]->getName();
        simField.units = _displayFields[ii]->getUnits();
        simField.minVal = _displayFields[ii]->getColorMap().rangeMin();
        simField.maxVal = _displayFields[ii]->getColorMap().rangeMax();
        simFields.push_back(simField);
      }
      simReader->setFields(simFields);
      
      break;
    }
      
    case Params::SIMULATED_INPUT:
    default: {
      
      SimReader *simReader = new SimReader(_params);
      _reader = simReader;
      
      vector<SimReader::Field> simFields;
      for (size_t ii = 0; ii < _displayFields.size(); ii++) {
        SimReader::Field simField;
        simField.name = _displayFields[ii]->getName();
        simField.units = _displayFields[ii]->getUnits();
        simField.minVal = _displayFields[ii]->getColorMap().rangeMin();
        simField.maxVal = _displayFields[ii]->getColorMap().rangeMax();
        simFields.push_back(simField);
      }
      simReader->setFields(simFields);

    }

  } // switch

  return 0;

}

//////////////////////////////////////////////////
// set up field objects, with their color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
  
int HawkEye::_setupDisplayFields()
{

  // we interleave unfiltered fields and filtered fields

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {

    const Params::field_t &pfld = _params._fields[ifield];

    // check we have a valid label
    
    if (strlen(pfld.label) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty field label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.raw_name) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty raw field name, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // create color map

    string colorMapPath = _params.color_scale_dir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    map.setName(pfld.label);
    map.setUnits(pfld.units);
    if (map.readMap(colorMapPath)) {
      cerr << "ERROR - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Cannot read in color map file: " << colorMapPath << endl;
      return -1;
    }
    
    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.label, pfld.raw_name, pfld.units, 
                       pfld.shortcut, map, ifield, false);
    
    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
        new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut, 
                         map, ifield, true);
      _displayFields.push_back(filt);
    }

  } // ifield

  if (_displayFields.size() < 1) {
    cerr << "ERROR - HawkEye::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  return 0;

}


///////////////////////////////////////////////////
// get the archive url

string HawkEye::_getArchiveUrl(const string &filePath)
  
{

  // find first digit in path - if no digits, return now
  
  const char *start = NULL;
  for (size_t ii = 0; ii < filePath.size(); ii++) {
    if (isdigit(filePath[ii])) {
      start = filePath.c_str() + ii;
      break;
    }
  }
  if (!start) {
    return "";
  }

  const char *end = start + strlen(start);

  // get day dir
  
  int year, month, day;
  while (start < end - 6) {
    if (sscanf(start, "%4d%2d%2d/", &year, &month, &day) == 3) {
      int urlLen = start - filePath.c_str() - 1;
      string url(filePath.substr(0, urlLen));
      if (_params.debug) {
        cerr << "===>> Setting archive url to: " << url << endl;
      }
      return url;
    }
    start++;
  }

  return "";

}

