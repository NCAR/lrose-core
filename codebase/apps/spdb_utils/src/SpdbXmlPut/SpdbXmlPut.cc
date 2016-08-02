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
// SpdbXmlPut.cc
//
// SpdbXmlPut object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <dsserver/DmapAccess.hh>
#include <didss/DsURL.hh>

#include <string>
#include <vector>
#include <cerrno>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaXml.hh>
#include <Spdb/DsSpdb.hh>
#include "SpdbXmlPut.hh"
using namespace std;

// Constructor

SpdbXmlPut::SpdbXmlPut(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "SpdbXmlPut";
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

  return;

}

// destructor

SpdbXmlPut::~SpdbXmlPut()

{

}

//////////////////////////////////////////////////
// Run

int SpdbXmlPut::Run ()
{

  // set up data types
  
  _computeDataType(_params.datatype, _dataType);
  _computeDataType(_params.datatype2, _dataType2);

  // run depending on mode

  if (_params.mode == Params::FILELIST) {
    return _runFilelistMode();
  } else {
    return _runCommandlineMode();
  }

}

//////////////////////////////////////////////////
// Run in command line mode

int SpdbXmlPut::_runCommandlineMode()
{

  // initialize SPDB

  DsSpdb spdb;
  spdb.setAppName(_progName);
  spdb.setPutMode(Spdb::putModeOver);
  
  // build up XML string
  
  string xml;
  if (_args.xml.size() > 0) {
    xml = _args.xml;
  } else {
    xml += TaXml::writeStartTag(_params.outer_xml_tag, 0);
    for (size_t ii = 0; ii < _args.tags.size(); ii++) {
      xml += TaXml::writeString(_args.tags[ii], 1, _args.vals[ii]);
    }
    xml += TaXml::writeEndTag(_params.outer_xml_tag, 0);
  }
  
  // add chunk
  
  spdb.addPutChunk(_dataType,
                   _args.validTime,
                   _args.validTime + _params.expire_secs,
                   xml.size() + 1,
                   xml.c_str(),
                   _dataType2);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "***************************" << endl;
    cerr << "Adding chunk: " << endl;
    cerr << "  Valid time: " << DateTime::str(_args.validTime) << endl;
    cerr << "  xml: " << xml << endl;
      cerr << "***************************" << endl;
  }
  
  // write to spdb

  if (spdb.put(_params.output_url, _params.spdb_id, _params.spdb_label)) {
    cerr << "ERROR - SpdbXmlPut::_runCommandLine" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote chunk to url: " << _params.output_url << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Run in filelist mode

int SpdbXmlPut::_runFilelistMode()
{

  int iret = 0;

  for (size_t ii = 0; ii < _args.inputFileList.size(); ii++) {
    if (_processFile(_args.inputFileList[ii])) {
      cerr << "ERROR - SpdbXmlPut::_runFilelistMode" << endl;
      cerr << "  Cannot process file: " << _args.inputFileList[ii] << endl;
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// process specified file

int SpdbXmlPut::_processFile (const string &filePath)

{
  
  if (_params.debug) {
    cerr << "==>> processing file: " << filePath << endl;
  }

  // initialize SPDB

  DsSpdb spdb;
  spdb.setAppName(_progName);
  spdb.setPutMode(Spdb::putModeOver);
  int nPutChunks = 0;

  // open file

  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopen(filePath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
    cerr << "  Cannot open file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in first line

  char line[65536];
  if (fgets(line, 65536, in) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
    cerr << "  Cannot read file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  if (strlen(line) < 2 || line[0] != _params.comment_character[0]) {
    cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
    cerr << "  Cannot read file: " << filePath << endl;
    cerr << "  First line must be label list, starting with: "
         << _params.comment_character[0] << endl;
    return -1;
  }

  // remove \n

  line[strlen(line)-1] = '\0';

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Label line: " << line << endl;
  }

  // tokenize the first line to get column headers
  // start at second character

  vector<string> labels;
  TaStr::tokenize(line + 1, _params.column_delimiter, labels);
  if (labels.size() < 2) {
    cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
    cerr << "  Cannot tokenize first line of file: " << filePath << endl;
    cerr << "  First line must be label list, starting with: "
         << _params.comment_character[0] << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < labels.size(); ii++) {
      cerr << "  ==>> label[" << ii << "]: " << labels[ii] << endl;
    }
  }

  // get location of date/time columns
  
  int yearCol = -1;
  int monthCol = -1;
  int dayCol = -1;
  int hourCol = -1;
  int minCol = -1;
  int secCol = -1;
  int yyyymmddhhmmssCol = -1;
  int unixTimeCol = -1;
  vector<int> valCols;
  
  for (size_t ii = 0; ii < labels.size(); ii++) {
    if (labels[ii] == "yyyymmddhhmmss") {
      yyyymmddhhmmssCol = ii;
    } else if (labels[ii] == "unix-time") {
      unixTimeCol = ii;
    } else if (labels[ii] == "year") {
      yearCol = ii;
    } else if (labels[ii] == "month") {
      monthCol = ii;
    } else if (labels[ii] == "day") {
      dayCol = ii;
    } else if (labels[ii] == "hour") {
      hourCol = ii;
    } else if (labels[ii] == "min") {
      minCol = ii;
    } else if (labels[ii] == "sec") {
      secCol = ii;
    } else {
      valCols.push_back(ii);
    }
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>>  yyyymmddhhmmssCol: " << yyyymmddhhmmssCol << endl;
    cerr << "==>>  unixTimeCol: " << unixTimeCol << endl;
    cerr << "==>>  yearCol: " << yearCol << endl;
    cerr << "==>>  monthCol: " << monthCol << endl;
    cerr << "==>>  dayCol: " << dayCol << endl;
    cerr << "==>>  hourCol: " << hourCol << endl;
    cerr << "==>>  minCol: " << minCol << endl;
    cerr << "==>>  secCol: " << secCol << endl;
    for (size_t ii = 0; ii < valCols.size(); ii++) {
      cerr << "==>>  valCols[" << ii << "]: " << valCols[ii] << endl;
    }
  }

  if (yyyymmddhhmmssCol < 0 && unixTimeCol < 0) {
    if (yearCol < 0 || monthCol < 0 || dayCol < 0 ||
        hourCol < 0 || minCol < 0 || secCol < 0) {
      cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
      cerr << "  date/time columns missing from first line: " << line << endl;
      cerr << "  You need: " << endl;
      cerr << "    'yyyymmddhhmmss' col or" << endl;
      cerr << "    'unix-time' col or" << endl;
      cerr << "    'year', 'month', 'day', 'hour', 'min', 'sec' cols" << endl;
      return -1;
    }
  }
  
  // go through file reading a line at a time

  while (!feof(in)) {

    // read another line
    
    if (fgets(line, 65536, in) == NULL) {
      if (!feof(in)) {
        int errNum = errno;
        cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
        cerr << "  Reading line from file: " << filePath << endl;
        cerr << "  " << strerror(errNum) << endl;
        return -1;
      }
    }
    
    // check len
    
    if (strlen(line) < 2) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Ignoring short line: " << line;
      }
      continue;
    }
    
    // remove \n
    
    line[strlen(line)-1] = '\0';
    
    // skip comments
    
    if (line[0] == _params.comment_character[0]) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Ignoring comment line: " << line << endl;
      }
      continue;
    }
    
    // tokenize line
    
    vector<string> cols;
    TaStr::tokenize(line, _params.column_delimiter, cols);

    // check number of cols

    if (cols.size() != labels.size()) {
      if (_params.debug) {
        cerr << "WARNING - ignoring line: " << line;
        cerr << "  Wrong number of tokens: " << cols.size() << endl;
        cerr << "  Should be: " << labels.size() << endl;
      }
      continue;
    }

    // get date/time
    
    DateTime dtime;
    int year, month, day, hour, min, sec;
    time_t unixTime = 0;
    
    if (yyyymmddhhmmssCol >= 0) {
      if (sscanf(cols[yyyymmddhhmmssCol].c_str(),
                 "%4d%2d%2d%2d%2d%2d",
                 &year, &month, &day, &hour, &min, &sec) == 6) {
        dtime.set(year, month, day, hour, min, sec);
        unixTime = dtime.utime();
      }
    }
    if (unixTime == 0 && unixTimeCol >= 0) {
      if (sscanf(cols[unixTimeCol].c_str(),
                 "%ld", &unixTime) == 1) {
        dtime.set(unixTime);
      }
    }
    if (unixTime == 0) {
      if ((sscanf(cols[yearCol].c_str(),"%d", &year) == 1) &&
          (sscanf(cols[monthCol].c_str(),"%d", &month) == 1) &&
          (sscanf(cols[dayCol].c_str(),"%d", &day) == 1) &&
          (sscanf(cols[hourCol].c_str(),"%d", &hour) == 1) &&
          (sscanf(cols[minCol].c_str(),"%d", &min) == 1) &&
          (sscanf(cols[secCol].c_str(),"%d", &sec) == 1)) {
        dtime.set(year, month, day, hour, min, sec);
        unixTime = dtime.utime();
      }
    }

    if (unixTime == 0) {
      if (_params.debug) {
        cerr << "WARNING - ignoring line: " << line;
        cerr << "  No date/time fields found" << endl;
      }
      continue;
    }

    // loop through the cols, building up XML string

    string xml;
    xml += TaXml::writeStartTag(_params.outer_xml_tag, 0);
    for (size_t ii = 0; ii < valCols.size(); ii++) {
      xml += TaXml::writeString(labels[valCols[ii]], 1,
                                cols[valCols[ii]]);
    }
    xml += TaXml::writeEndTag(_params.outer_xml_tag, 0);

    // add chunk
    
    spdb.addPutChunk(_dataType,
                     unixTime,
                     unixTime + _params.expire_secs,
                     xml.size() + 1,
                     xml.c_str(),
                     _dataType2);
    nPutChunks++;

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "***************************" << endl;
      cerr << "Adding chunk: " << endl;
      cerr << "  Valid time: " << DateTime::str(unixTime) << endl;
      cerr << "  xml: " << xml << endl;
      cerr << "***************************" << endl;
    }

  } // while

  // close input file

  inFile.fclose();

  if (_params.debug) {
    cerr << "Writing " << nPutChunks << " entries to url: " 
         << _params.output_url << endl;
  }

  // write to spdb

  if (spdb.put(_params.output_url, _params.spdb_id, _params.spdb_label)) {
    cerr << "ERROR - SpdbXmlPut::_processFile" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Success, wrote to url: " << _params.output_url << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Compute data types

void SpdbXmlPut::_computeDataType(const char *str,
                                  int &datatype)

{

  // check for numeric

  bool numeric = true;
  for (size_t ii = 0; ii < strlen(str); ii++) {
    if (!isdigit(str[ii])) {
      numeric = false;
    }
  }

  if (numeric) {
    datatype = atoi(str);
  } else {
    datatype = Spdb::hash4CharsToInt32(str);
  }

}


