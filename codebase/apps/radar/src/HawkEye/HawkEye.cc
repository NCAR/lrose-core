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
#include <qtplot/ColorMap.hh>
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"
#include "SoloDefaultColorWrapper.hh"
#include <toolsa/Path.hh>
#include <toolsa/LogStream.hh>
//#include <toolsa/Log.hh>
//#include <toolsa/MsgLog.hh>
#include "HawkEyeLogger.hh"

#include <string>
#include <iostream>
#include <QApplication>

using namespace std;

// Constructor

HawkEye::HawkEye(int argc, char **argv) :
        _args("HawkEye")

{

  OK = true;
  _polarManager = NULL;
  _bscanManager = NULL;
  _reader = NULL;

  // set programe name

  _progName = strdup("HawkEye");

  HawkEyeLogger logger("HawkEye");
  logger.setDayMode();
  logger.setOutputDir("/tmp/Applications/HawkEye/Logs");
  logger.openFile();
  logger.postLine("HawkEye starting");
  logger.closeFile();

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

  if (_params.debug) {
    LOG_STREAM_INIT(true, false, true, true);
  } else {
    LOG_STREAM_INIT(false, false, false, false);
    LOG_STREAM_TO_CERR();
  }

  // print color scales if debugging
  if (_params.debug) {
    SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
    sd.PrintColorScales();
  } 

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
  // logger.closeFile();

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
      _polarManager->setArchiveFileList(_args.inputFileList);
      // override archive data url from input file
      string url = _getArchiveUrl(_args.inputFileList[0]);
      TDRP_str_replace(&_params.archive_data_url, url.c_str());
    } else if (_params.begin_in_archive_mode) {
      if (_polarManager->loadArchiveFileList()) {
        
        string errMsg = "WARNING\n";
        errMsg.append("<p>HawkEye cannot find archive data files. </p>");
        errMsg.append("<p> Choose a file to open or change the time limits. </p>");
        //errMsg.append(" in startup location. </p>");
        //errMsg.append(_params.archive_data_url);
        //errMsg.append(")</p>");
        //errMsg.append("<p> Click OK to continue to use HawkEye.</p>");
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();

        // return -1;
      }
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

      _params.begin_in_archive_mode = pFALSE;

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

  // check for color map location
  
  string colorMapDir = _params.color_scale_dir;
  Path mapDir(_params.color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params.color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - HawkEye" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params.color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

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
    
    string colorMapPath = colorMapDir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    map.setName(pfld.label);
    map.setUnits(pfld.units);
    // TODO: the logic here is a little weird ... the label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath)) {
        cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
        cerr << "  Cannot read in color map file: " << colorMapPath << endl;
        cerr << "  Looking for default color map for field " << pfld.label << endl; 

        try {
          // check here for smart color scale; look up by field name/label and
          // see if the name is a usual parameter for a known color map
          SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
          ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.label);
          cerr << "  found default color map for " <<  pfld.label  << endl;
          // if (_params.debug) colorMap.print(cout); // LOG(DEBUG_VERBOSE)); // cout);
          map = colorMap;
          // HERE: What is missing from the ColorMap object??? 
        } catch (std::out_of_range &ex) {
          cerr << "WARNING - did not find default color map for field; using rainbow colors" << endl;
	  // Just set the colormap to a generic color map
	  // use range to indicate it needs update; update when we have access to the actual data values
          map = ColorMap(0.0, 1.0);
	  noColorMap = true; 
          // return -1
        }
    }

    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.label, pfld.raw_name, pfld.units, 
                       pfld.shortcut, map, ifield, false);
    if (noColorMap)
      field->setNoColorMap();

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

