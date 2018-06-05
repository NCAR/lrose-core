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
// SpolWarnSms.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////
//
// SpolWarnSms reads status XML monitoring data from SPDB,
// for the S-band and K-band.
// It checks the status in the XML, and determines if
// a warning message should be sent
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <dsserver/DsLdataInfo.hh>
#include "SpolWarnSms.hh"

using namespace std;

// Constructor

SpolWarnSms::SpolWarnSms(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "SpolWarnSms";
  ucopyright(_progName.c_str());

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
  
  // SPDB if needed

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _spdbSband.setDebug();
    _spdbKband.setDebug();
    if (_params.write_to_spdb) {
      _spdbOut.setDebug();
    }
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

}

// destructor

SpolWarnSms::~SpolWarnSms()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolWarnSms::Run ()
{
  
  PMU_auto_register("Run");
  
  while (true) {
    
    PMU_auto_register("Getting status");
    time_t now = time(NULL);

    // read status

    // check for warning conditions

    // write message for SMS

    // update spdb?

    if (_params.write_to_spdb) {
      _updateSpdb(now);
    }

  } // while
  
  return 0;
  
}

/////////////////////////////
// update the SPDB data base

int SpolWarnSms::_updateSpdb(time_t now)

{

  if (_params.debug) {
    cerr << "==>> updating SPDB" << endl;
  }
  
  // get the concatenated xml string
  
  // string xml = _getCombinedXml(now);
  string xml;
  
  if (xml.size() == 0) {
    return 0;
  }
  
  _spdbOut.clearPutChunks();
  _spdbOut.addPutChunk(0, now, now + _params.monitoring_interval_secs,
                       xml.size() + 1, xml.c_str());
  
  if (_spdbOut.put(_params.spdb_url,
                   SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - SpolWarnSms::_updateSpdb" << endl;
    cerr << _spdbOut.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote SPDB data to: " << _params.spdb_url << endl;
  }

  return 0;

}

/////////////////////////////
// check status

int SpolWarnSms::_checkStatus(time_t now)

{

  if (_params.debug) {
    cerr << "==>> Checking status" << endl;
  }

  // get the concatenated xml string

  // string xml = _getCombinedXml(now);
  string xml;
  
  if (_params.debug) {
    cerr << "statusXml for nagios: " << endl;
    cerr << xml << endl;
  }

#ifdef JUNK

  // create directory as needed

  Path path(_params.nagios_file_path);
  if (ta_makedir_recurse(path.getDirectory().c_str())) {
    int errNum = errno;
    cerr << "ERROR - SpolWarnSms::_updateNagios" << endl;
    cerr << "  Cannot make directory for output file: " << _params.nagios_file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // write to tmp nagios file

  string tmpPath(_params.nagios_file_path);
  tmpPath += ".tmp";
  
  FILE *tmpFile = fopen(tmpPath.c_str(), "w");
  if (tmpFile == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolWarnSms::_updateNagios" << endl;
    cerr << "  Cannot open tmp file for writing: " << tmpPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // write to nagios file

  for (int ii = 0; ii < _params.xml_entries_n; ii++) {

    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    
    string sectionStr;
    if (TaXml::readString(xml, entry.xml_outer_tag, sectionStr)) {
      if (_params.debug >= Params::DEBUG_EXTRA) { 
        cerr << "WARNING - SpolWarnSms::_updateNagios" << endl;
        cerr << " Cannot find main tag: " << entry.xml_outer_tag << endl;
      }
    }

    switch (entry.entry_type) {
      
      case Params::XML_ENTRY_BOOLEAN:
        _handleBooleanNagios(sectionStr, entry, tmpFile);
        break;
        
      case Params::XML_ENTRY_BOOLEAN_TO_INT:
        _handleBooleanToIntNagios(sectionStr, entry, tmpFile);
        break;
        
      case Params::XML_ENTRY_INT:
        _handleIntNagios(sectionStr, entry, tmpFile);
        break;
        
      case Params::XML_ENTRY_DOUBLE:
        _handleDoubleNagios(sectionStr, entry, tmpFile);
        break;
        
      case Params::XML_ENTRY_STRING:
      default:
        _handleStringNagios(sectionStr, entry, tmpFile);
        break;
        
    } // switch (entry.entry_type)
        
  } // ii

  if (_params.nagios_monitor_antenna_movement) {
    _addMovementToNagios(tmpFile);
  }

  fclose(tmpFile);
  
  // rename file

  if (rename(tmpPath.c_str(), _params.nagios_file_path)) {
    int errNum = errno;
    cerr << "ERROR - SpolWarnSms::_updateNagios" << endl;
    cerr << "  Cannot rename tmp file to final path" << endl;
    cerr << "  tmp path: " << tmpPath << endl;
    cerr << "  final path: " << _params.nagios_file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote Nagios data to: " << _params.nagios_file_path << endl;
  }

#endif

  return 0;

}

///////////////////////////////////////////
// handle a boolean entry in the status xml

int SpolWarnSms::_handleBooleanEntry(const string &xml,
                                     const Params::xml_entry_t &entry,
                                     FILE *outputFile)
  
{
  
  string label = entry.label;
  if (label.size() == 0) {
    label = entry.xml_tags;
  }

#ifdef JUNK

  // get the substring

  string sval;
  if (TaXml::readString(xml, entry.xml_inner_tag, sval)) {
    _handleMissingEntry(xml, entry, outputFile);
    return 0;
  }

  // get the boolean value

  bool bval;
  if (TaXml::readBoolean(xml, entry.xml_inner_tag, bval)) {
    cerr << "ERROR - SpolWarnSms::_handleBooleanNagios" << endl;
    cerr << " Cannot find sub tag: " << entry.xml_inner_tag << endl;
    return -1;
  }
  
  int warnLevel = 0;
  string warnStr = "OK";
  if (bval == entry.ok_boolean) {
    warnLevel = 0;
    warnStr = "OK";
  } else {
    if (entry.boolean_failure_is_critical) {
      warnLevel = 2;
      warnStr = "CRITICAL";
    } else {
      warnLevel = 1;
      warnStr = "WARN";
    }
  }
  int warnLimit = 1;
  int critLimit = 1;
  if (entry.ok_boolean) {
    critLimit = 0;
    warnLimit = 0;
  }

  char comment[1024];
  if (strlen(entry.comment) > 0) {
    sprintf(comment, " (%s)", entry.comment);
  } else {
    comment[0] = '\0';
  }

  char text[4096];
  sprintf(text, "%d %s_%s %s=%d;%d;%d;%g;%g; %s - %s = %s %s\n",
          warnLevel,
          entry.xml_outer_tag,
          entry.xml_inner_tag,
          label.c_str(),
          bval,
          warnLimit,
          critLimit,
          entry.graph_min_val,
          entry.graph_max_val,
          warnStr.c_str(),
          label.c_str(),
          sval.c_str(),
          comment);
  
  fprintf(outputFile, "%s", text);
  if (_params.debug) {
    fprintf(stderr, "Adding nagios entry: ");
    fprintf(stderr, "%s", text);
  }

#endif

  return 0;

}

///////////////////////////////////////////
// handle a double entry in the status xml

int SpolWarnSms::_handleNumberEntry(const string &xml,
                                    const Params::xml_entry_t &entry,
                                    FILE *outputFile)
  
{

  string label = entry.label;
  if (label.size() == 0) {
    label = entry.xml_tags;
  }

#ifdef JUNK

  // get the substring
  
  string sval;
  if (TaXml::readString(xml, entry.xml_inner_tag, sval)) {
    _handleMissingEntry(xml, entry, outputFile);
    return 0;
  }

  double dval;
  if (TaXml::readDouble(xml, entry.xml_inner_tag, dval)) {
    if (_params.debug) { 
      cerr << "WARNING - SpolWarnSms::_handleBooleanNagios" << endl;
      cerr << " Cannot find sub tag: " << entry.xml_inner_tag << endl;
    }
    return -1;
  }
  
  int warnLevel = 0;
  string warnStr = "OK";
  if (dval >= entry.ok_value_lower_limit &&
      dval <= entry.ok_value_upper_limit) {
    warnLevel = 0;
    warnStr = "OK";
  } else if (dval >= entry.impaired_value_lower_limit &&
             dval <= entry.impaired_value_upper_limit) {
    warnLevel = 1;
    warnStr = "WARN";
  } else {
    warnLevel = 2;
    warnStr = "CRITICAL";
  }

  double warnLimit = entry.ok_value_upper_limit;
  double critLimit = entry.impaired_value_upper_limit;
  if (entry.impaired_value_lower_limit < entry.ok_value_lower_limit) {
    warnLimit = entry.ok_value_lower_limit;
    critLimit = entry.impaired_value_lower_limit;
  }

  string label = entry.label;
  if (label.size() == 0) {
    label = entry.xml_inner_tag;
  }

  char comment[1024];
  if (strlen(entry.comment) > 0) {
    sprintf(comment, " (%s)", entry.comment);
  } else {
    comment[0] = '\0';
  }

  char text[4096];
  sprintf(text, "%d %s_%s %s=%s;%g;%g;%g;%g; %s - %s = %s %s%s\n",
          warnLevel,
          entry.xml_outer_tag,
          entry.xml_inner_tag,
          label.c_str(),
          sval.c_str(),
          warnLimit,
          critLimit,
          entry.graph_min_val,
          entry.graph_max_val,
          warnStr.c_str(),
          label.c_str(),
          sval.c_str(),
          entry.units,
          comment);
  
  fprintf(outputFile, "%s", text);
  if (_params.debug) {
    fprintf(stderr, "Adding nagios entry: ");
    fprintf(stderr, "%s", text);
  }

#endif

  return 0;
  
}

