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
// RcalXml2Table.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2022
//
///////////////////////////////////////////////////////////////
//
// RcalXml2Table reads radar calibration files in XML format
// and writes out the data into a comma or space delimited
// text table.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/TaStr.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxRcalib.hh>
#include "RcalXml2Table.hh"

using namespace std;

// Constructor

RcalXml2Table::RcalXml2Table(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "RcalXml2Table";
  
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

  // set start and end times

  _startTime.set(_params.start_time.year,
                 _params.start_time.month,
                 _params.start_time.day,
                 _params.start_time.hour,
                 _params.start_time.min,
                 _params.start_time.sec);
  
  _endTime.set(_params.end_time.year,
               _params.end_time.month,
               _params.end_time.day,
               _params.end_time.hour,
               _params.end_time.min,
               _params.end_time.sec);

  if (_args.inputFileList.size() > 0) {
    _inputPath = new DsInputPath(_progName,
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _args.inputFileList);
  } else {
    _inputPath = new DsInputPath(_progName,
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _params.input_dir,
                                 _startTime.utime(),
                                 _endTime.utime());
  }

  _inputPath->setSearchExt("xml");

  // create the output field list
  
  _createFieldList();
  _lineCount = 0;

}

// destructor

RcalXml2Table::~RcalXml2Table()

{

  delete _inputPath;
  
}

//////////////////////////////////////////////////
// Run

int RcalXml2Table::Run ()
{
  
  // add comment lines is appropriate
  
  if (_params.add_commented_header) {
    _printComments(stdout);
  }
  
  // loop through the files

  char *path = NULL;
  while ((path = _inputPath->next()) != NULL) {

    cerr << "Working on file: " << path << endl;

    RadxRcalib cal;
    string errStr;
    if (cal.readFromXmlFile(path, errStr)) {
      cerr << "ERROR - reading XML cal file: " << path << endl;
      continue;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "========================================" << endl;
      cerr << "Read in cal file: " << path << endl;
      cal.print(cerr);
      cerr << "========================================" << endl;
    }
    
    _printLine(stdout, cal);

  } // while
  
  return 0;
  
}

//////////////////////////////////////////////////
// create field list

void RcalXml2Table::_createFieldList()
{

  _fieldList.clear();
  
  _fieldList.push_back("radarName");
  _fieldList.push_back("wavelengthCm");
  _fieldList.push_back("beamWidthDegH");
  _fieldList.push_back("beamWidthDegV");
  _fieldList.push_back("antGainDbH");
  _fieldList.push_back("antGainDbV");
  _fieldList.push_back("pulseWidthUs");
  _fieldList.push_back("xmitPowerDbmH");
  _fieldList.push_back("xmitPowerDbmV");
  _fieldList.push_back("twoWayWaveguideLossDbH");
  _fieldList.push_back("twoWayWaveguideLossDbV");
  _fieldList.push_back("twoWayRadomeLossDbH");
  _fieldList.push_back("twoWayRadomeLossDbV");
  _fieldList.push_back("receiverMismatchLossDb");
  _fieldList.push_back("kSquaredWater");
  _fieldList.push_back("radarConstH");
  _fieldList.push_back("radarConstV");
  _fieldList.push_back("noiseDbmHc");
  _fieldList.push_back("noiseDbmHx");
  _fieldList.push_back("noiseDbmVc");
  _fieldList.push_back("noiseDbmVx");
  _fieldList.push_back("i0DbmHc");
  _fieldList.push_back("i0DbmHx");
  _fieldList.push_back("i0DbmVc");
  _fieldList.push_back("i0DbmVx");
  _fieldList.push_back("receiverGainDbHc");
  _fieldList.push_back("receiverGainDbHx");
  _fieldList.push_back("receiverGainDbVc");
  _fieldList.push_back("receiverGainDbVx");
  _fieldList.push_back("receiverSlopeDbHc");
  _fieldList.push_back("receiverSlopeDbHx");
  _fieldList.push_back("receiverSlopeDbVc");
  _fieldList.push_back("receiverSlopeDbVx");
  _fieldList.push_back("dynamicRangeDbHc");
  _fieldList.push_back("dynamicRangeDbHx");
  _fieldList.push_back("dynamicRangeDbVc");
  _fieldList.push_back("dynamicRangeDbVx");
  _fieldList.push_back("baseDbz1kmHc");
  _fieldList.push_back("baseDbz1kmHx");
  _fieldList.push_back("baseDbz1kmVc");
  _fieldList.push_back("baseDbz1kmVx");
  _fieldList.push_back("sunPowerDbmHc");
  _fieldList.push_back("sunPowerDbmHx");
  _fieldList.push_back("sunPowerDbmVc");
  _fieldList.push_back("sunPowerDbmVx");
  _fieldList.push_back("noiseSourcePowerDbmH");
  _fieldList.push_back("noiseSourcePowerDbmV");
  _fieldList.push_back("powerMeasLossDbH");
  _fieldList.push_back("powerMeasLossDbV");
  _fieldList.push_back("couplerForwardLossDbH");
  _fieldList.push_back("couplerForwardLossDbV");
  _fieldList.push_back("dbzCorrection");
  _fieldList.push_back("zdrCorrectionDb");
  _fieldList.push_back("ldrCorrectionDbH");
  _fieldList.push_back("ldrCorrectionDbV");
  _fieldList.push_back("systemPhidpDeg");
  _fieldList.push_back("testPowerDbmH");

}

//////////////////////////////////////////////////
// print comments at start

void RcalXml2Table::_printComments(FILE *out)
{

  const char *com = _params.comment_character;
  const char *delim = _params.column_delimiter;

  // initial line has column headers

  fprintf(out, "%scount", com);
  fprintf(out, "%syear", delim);
  fprintf(out, "%smonth", delim);
  fprintf(out, "%sday", delim);
  fprintf(out, "%shour", delim);
  fprintf(out, "%smin", delim);
  fprintf(out, "%ssec", delim);
  fprintf(out, "%sunix_time", delim);
  fprintf(out, "%sradarName", delim);

  // then comment lines

  fprintf(out, "%s============================================\n", com);
  fprintf(out, "%s  Table produced by RcalXml2Table\n", com);
  fprintf(out, "%s  dir: %s\n", com, _params.input_dir);
  fprintf(out, "%s  start_time: %s\n", com,
          DateTime::strm(_startTime.utime()).c_str());
  fprintf(out, "%s  end_time: %s\n", com,
          DateTime::strm(_endTime.utime()).c_str());
  fprintf(out, "%s  ----------- Table column list ------------\n", com);
  int colNum = 0;
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "count");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "year");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "month");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "day");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "hour");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "min");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "sec");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "unix_time");
  fprintf(out, "%s    col %.3d: %s\n", com, colNum++, "unix_day");
  for (size_t ii = 0; ii < _fieldList.size(); ii++) {
    fprintf(out, "%s    col %.3d: %s\n",
            com, colNum++, _fieldList[ii].c_str());
  } // ii
  fprintf(out, "%s  ------------------------------------------\n", com);
  fprintf(out, "%s============================================\n", com);

}

//////////////////////////////////////////////////
// print a line of data

void RcalXml2Table::_printLine(FILE *out,
                              const RadxRcalib &cal)
  
{

  char text[8192];
  string ostr;
  
  // date and time
  
  DateTime vtime(cal.getCalibTime());
  snprintf(text, 8192, "%8ld%s%.4d%s%.2d%s%.2d%s%.2d%s%.2d%s%.2d%s%ld%s%12.6f",
           _lineCount, _params.column_delimiter,
           vtime.getYear(), _params.column_delimiter,
           vtime.getMonth(), _params.column_delimiter,
           vtime.getDay(), _params.column_delimiter,
           vtime.getHour(), _params.column_delimiter,
           vtime.getMin(), _params.column_delimiter,
           vtime.getSec(), _params.column_delimiter,
           (long) vtime.utime(), _params.column_delimiter,
           (double) vtime.utime() / 86400.0);
  ostr += text;
  
  // set the xml string from the cal object
  
  string xml;
  cal.convert2Xml(xml);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===>> time: " << DateTime::strm(vtime.utime()) << endl;
    cerr << "==================== XML =====================" << endl;
    cerr << xml;
    cerr << "==============================================" << endl;
  }
  
  // loop through XML entries

  for (size_t ii = 0; ii < _fieldList.size(); ii++) {

    string fieldName = _fieldList[ii];
    
    // delimiter

    snprintf(text, 8192, "%s", _params.column_delimiter);
    ostr += text;

    // value
    
    string val;
    if (RadxXml::readString(xml, fieldName, val)) {
      ostr += "nan";
    } else {
      ostr += val;
    }

  } // ii

  fprintf(out, "%s\n", ostr.c_str());
  _lineCount++;

}

