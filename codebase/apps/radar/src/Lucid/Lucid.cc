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
// Lucid.cc
//
// Lucid display
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// Lucid is the Qt replacement for CIDD
//
///////////////////////////////////////////////////////////////

#include "Lucid.hh"
#include "CartManager.hh"
// #include "DisplayField.hh"
#include "LegacyParams.hh"
#include "SoloDefaultColorWrapper.hh"
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <qtplot/ColorMap.hh>

#include <string>
#include <iostream>
#include <QApplication>
#include <QErrorMessage>

#define THIS_IS_MAIN 1 /* This is the main module */
#include "cidd.h"

using namespace std;

// Constructor

Lucid::Lucid(int argc, char **argv) :
        _args("Lucid")

{

  OK = true;
  _cartManager = NULL;

  // set programe name

  _progName = strdup("Lucid");

  // initialize legacy CIDD structs
  
  init_globals();

  // initialize signal handling
  
  init_signal_handlers();  

  // check for legacy params file
  // if found create a temporary tdrp file based on the legacy file

  string legacyParamsPath;
  char tdrpParamsPath[5000];
  bool usingLegacyParams = false;
  if (_args.getLegacyParamsPath(argc, (const char **) argv, legacyParamsPath) == 0) {
    // gd.db_name = strdup(legacyParamsPath.c_str());
    Path lpPath(legacyParamsPath);
    snprintf(tdrpParamsPath, 4999,
             "/tmp/Lucid.%s.%d.tdrp", lpPath.getFile().c_str(), getpid());
    LegacyParams lParams;
    lParams.translateToTdrp(legacyParamsPath, tdrpParamsPath);
    usingLegacyParams = true;
  }

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }
  
  // load TDRP params from command line

  char *paramsPath = (char *) "unknown";
  if (usingLegacyParams) {
    if (_params.loadApplyArgs(tdrpParamsPath,
                              argc, argv,
                              _args.override.list)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
    _paramsPathRequested = legacyParamsPath;
    _paramsPathUsed = tdrpParamsPath;
  } else {
    if (_params.loadFromArgs(argc, argv,
                             _args.override.list,
                             &paramsPath)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters." << endl;
      OK = false;
      return;
    }
    _paramsPathRequested = paramsPath;
    _paramsPathUsed = paramsPath;
  }

  if (_params.debug) {
    cerr << "Using params path: " << _paramsPathUsed.getPath() << endl;
  }
  
  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }
    
    // initialize globals, get/set defaults, establish data sources etc.

  if (init_data_space()) {
    OK = false;
    return;
  }

  // print color scales if debugging
  // if (_params.debug) {
  //   SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
  //   sd.PrintColorScales();
  // } 

  // set up display fields

  // if (_setupDisplayFields()) {
  //   OK = false;
  //   return;
  // }

  // get the display
  
  // if (_setupXDisplay(argc, argv)) {
  //   cerr << "Cannot set up X display" << endl;
  //   OK = false;
  // }
  
  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

Lucid::~Lucid()

{

  if (_cartManager) {
    delete _cartManager;
  }

  // if (_bscanManager) {
  //   delete _bscanManager;
  // }

  // for (size_t ii = 0; ii < _displayFields.size(); ii++) {
  //   delete _displayFields[ii];
  // }
  // _displayFields.clear();

}

//////////////////////////////////////////////////
// Run

int Lucid::Run(QApplication &app)
{

  /* Establish color table & mappings  */

  // setup_colorscales(gd.dpy);
  
  // Instantiate Symbolic products

  // init_symprods();

  /* make changes to objects for gui */
  
  // modify_gui_objects();
  
  gd.finished_init = 1;

  // create cartesian display
  
  _cartManager = new CartManager;
  return _cartManager->run(app);
  
}

#ifdef NOTNOW

//////////////////////////////////////////////////
// set up the display variable
  
int Lucid::_setupXDisplay(int argc, char **argv)
{

  /*
   * search for display name on command line
   */

  gd.dpyName = NULL;
  
  for (int i =  1; i < argc; i++) {
    if (!strcmp(argv[i], "-display") || !strcmp(argv[i], "-d")) {
      if (i < argc - 1) {
	gd.dpyName = new char[strlen(argv[i+1]) + 1];
	strcpy(gd.dpyName, argv[i+1]);
      }
    }
  } /* i */
	
  if((gd.dpy = XOpenDisplay(gd.dpyName)) == NULL) {
    fprintf(stderr, "ERROR - Lucid::_setupXDisplay\n");
    fprintf(stderr,
	    "Cannot open display '%s' or '%s'\n",
	    gd.dpyName, getenv("DISPLAY"));
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// set up field objects, with their color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
  
int Lucid::_setupDisplayFields()
{

  // check for color map location
  
  string colorMapDir = _params.color_scale_urls;
  Path mapDir(colorMapDir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params.color_scale_urls);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - Lucid" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params.color_scale_urls << endl;
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
    
    if (strlen(pfld.legend_label) == 0) {
      cerr << "WARNING - Lucid::_setupDisplayFields()" << endl;
      cerr << "  Empty field legend_label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.field_name) == 0) {
      cerr << "WARNING - Lucid::_setupDisplayFields()" << endl;
      cerr << "  Empty raw field name, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }

    // create color map
    
    string colorMapPath = colorMapDir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    // map.setName(pfld.legend_label);
    map.setName(pfld.color_map);
    map.setUnits(pfld.field_units);
    cerr << "MMMMMMMMMMMMMMMM colorMap name: " << pfld.color_map << endl;
    // TODO: the logic here is a little weird ... the legend_label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath) == 0) {
      
      cerr << "NNNNNNNNNNNNNNNNNNNNNNNN colorMap name: " << pfld.color_map << endl;
      
      map.setName(pfld.color_map);

    } else {

      cerr << "WARNING - Lucid::_setupDisplayFields()" << endl;
      cerr << "  Cannot read in color map file: " << colorMapPath << endl;
      cerr << "  Looking for default color map for field " << pfld.legend_label << endl; 
      try {
        // check here for smart color scale; look up by field name/legend_label and
        // see if the name is a usual parameter for a known color map
        SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
        ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.legend_label);
        cerr << "  found default color map for " <<  pfld.legend_label  << endl;
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

    } // if (map.readMap(colorMapPath)

    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.legend_label, pfld.field_name, pfld.field_units, 
                       "a", map, ifield, false);
    if (noColorMap) {
      field->setNoColorMap();
    }

    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.field_name) > 0) {
      string filtField_Label = string(pfld.legend_label) + "-filt";
      DisplayField *filt =
        new DisplayField(pfld.legend_label, pfld.field_name, pfld.field_units, "a", 
                         map, ifield, true);
      _displayFields.push_back(filt);
    }

  } // ifield

  if (_displayFields.size() < 1) {
    cerr << "ERROR - Lucid::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  return 0;

}

#endif
