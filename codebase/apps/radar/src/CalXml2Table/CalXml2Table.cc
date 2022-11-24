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
// CalXml2Table.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2022
//
///////////////////////////////////////////////////////////////
//
// CalXml2Table reads radar calibration files in XML format
// and writes out the data into a comma or space delimited
// text table.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include "CalXml2Table.hh"

using namespace std;

// Constructor

CalXml2Table::CalXml2Table(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "CalXml2Table";
  
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
  
}

// destructor

CalXml2Table::~CalXml2Table()

{

  delete _inputPath;
  
}

//////////////////////////////////////////////////
// Run

int CalXml2Table::Run ()
{
  
  // add comment lines is appropriate
  
  if (_params.add_commented_header) {
    _printComments(stdout);
  }
  
  // loop through the files

  char *path = NULL;
  while ((path = _inputPath->next()) != NULL) {
    cerr << "Working on file: " << path << endl;
  }

#ifdef JUNK
  
  time_t startSecs = _startTime.utime();
  time_t endSecs = _endTime.utime();
  
  long long startDay = startSecs / 86400;
  long long endDay = endSecs / 86400;

  if (_params.debug) {
    cerr << "global start time: " << DateTime::strm(startSecs) << endl;
    cerr << "global end   Time: " << DateTime::strm(endSecs) << endl;
  }

  for (long long iday = startDay; iday <= endDay; iday++) {

    DsSpdb spdb;
    
    time_t startTime = iday * 86400;
    time_t endTime = startTime + 86399;

    if (startTime < startSecs) {
      startTime = startSecs;
    }
    if (endTime > endSecs) {
      endTime = endSecs;
    }
    
    if (_params.debug) {
      cerr << "  Working on day: " << iday << endl;
      cerr << "  day start time: " << DateTime::strm(startTime) << endl;
      cerr << "  day end   time: " << DateTime::strm(endTime) << endl;
    }

    if (spdb.getInterval(_params.input_url,
                         startTime,
                         endTime,
                         _params.data_type,
                         _params.data_type_2)) {
      cerr << "ERROR - CalXml2Table::Run" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
    
    // get chunks
    
    const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
    if (_params.debug) {
      cerr << "==>> got n entries: " << chunks.size() << endl;
    }
    for (size_t ii = 0; ii < chunks.size(); ii++) {
      _printLine(stdout, chunks[ii]);
    }

  }

#endif

  return 0;
  
}

//////////////////////////////////////////////////
// print comments at start

void CalXml2Table::_printComments(FILE *out)
{

#ifdef JUNK
  
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
  fprintf(out, "%sunix_day", delim);

  for (int ii = 0; ii < _params.xml_entries_n; ii++) {
    fprintf(out, "%s", delim);
    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    if (entry.specify_label) {
      fprintf(out, "%s", entry.label);
    } else {
      fprintf(out, "%s", entry.xml_tag_list);
    }
  } // ii
  fprintf(out, "\n");

  // then comment lines

  fprintf(out, "%s============================================\n", com);
  fprintf(out, "%s  Table produced by CalXml2Table\n", com);
  fprintf(out, "%s  url: %s\n", com, _params.input_url);
  fprintf(out, "%s  start_time: %s\n", com,
          DateTime::strm(_startTime.utime()).c_str());
  fprintf(out, "%s  end_time: %s\n", com,
          DateTime::strm(_endTime.utime()).c_str());
  if (_params.data_type != 0) {
    fprintf(out, "%s  data_type: %d\n", com, _params.data_type);
  }
  if (_params.data_type_2 != 0) {
    fprintf(out, "%s  data_type_2: %d\n", com, _params.data_type_2);
  }
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
  for (int ii = 0; ii < _params.xml_entries_n; ii++) {
    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    if (entry.specify_label) {
      fprintf(out, "%s    col %.3d: %s\n",
              com, colNum++, entry.label);
    } else {
      fprintf(out, "%s    col %.3d: %s\n",
              com, colNum++, entry.xml_tag_list);
    }
  } // ii
  fprintf(out, "%s  ------------------------------------------\n", com);
  fprintf(out, "%s============================================\n", com);

#endif
  
}

//////////////////////////////////////////////////
// print a line of data

void CalXml2Table::_printLine(FILE *out,
                              const RadxRcalib &cal)
  
{

#ifdef JUNK
  
  if (chunk.data == NULL) {
    return;
  }

  char text[8192];
  string ostr;

  // date and time

  DateTime vtime(chunk.valid_time);
  snprintf(text, 8192, "%8d%s%.4d%s%.2d%s%.2d%s%.2d%s%.2d%s%.2d%s%ld%s%12.6f",
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
  
  // set the xml string from the chunk data

  string xml((const char *) chunk.data);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "===>> time: " << DateTime::strm(vtime.utime()) << endl;
    cerr << "==================== XML =====================" << endl;
    cerr << xml;
    cerr << "==============================================" << endl;
  }
  
  // loop through XML entries

  bool allNans = true;
  bool anyNans = false;
  
  for (int ii = 0; ii < _params.xml_entries_n; ii++) {
    
    // delimiter

    snprintf(text, 8192, "%s", _params.column_delimiter);
    ostr += text;

    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    
    // get tag list
    
    vector<string> tags;
    TaStr::tokenize(entry.xml_tag_list, "<>", tags);
    if (tags.size() == 0) {
      if (entry.required) {
        cerr << "NOTE: required entry not found, ignoring" << endl;
        cerr << "  ==>> label       : " << entry.label << endl;
        cerr << "  ==>> xml_tag_list: " << entry.xml_tag_list << endl;
        return;
      }
      snprintf(text, 8192, "nan");
      ostr += text;
      continue;
    }
    
    string buf(xml);
    for (size_t jj = 0; jj < tags.size(); jj++) {

      string val;

      if (TaXml::readString(buf, tags[jj], val)) {
        if (entry.required) {
          cerr << "ERROR - required entry not found, ignoring" << endl;
          cerr << "  ==>> label       : " << entry.label << endl;
          cerr << "  ==>> xml_tag_list: " << entry.xml_tag_list << endl;
          return;
        }
        snprintf(text, 8192, "nan");
        ostr += text;
        break;
      }

      if (jj == (tags.size() - 1)) {

        if (val.find("nan") != 0) {
          allNans = false;
        } else {
          anyNans = true;
        }
        if (_params.replace_string_in_output) {
          size_t pos = 0;
          while (pos != string::npos) {
            pos = val.find(_params.old_string, 0);
            if (pos != string::npos) {
              val.replace(pos, strlen(_params.old_string), _params.new_string);
            }
          } // while
        } // if (_params.replace_string_in_output) 
        if (_params.convert_boolean_to_integer) {
          if (STRequal(val.c_str(), "TRUE")) {
            snprintf(text, 8192, "1");
            ostr += text;
          } else if (STRequal(val.c_str(), "FALSE")) {
            snprintf(text, 8192, "0");
            ostr += text;
          } else {
            snprintf(text, 8192, "%s", val.c_str());
            ostr += text;
          }
        } else {
          snprintf(text, 8192, "%s", val.c_str());
          ostr += text;
        }

      } else {

        buf = val;

      } // if (jj == (tags.size() - 1)) 

    } // jj

  } // ii
  
  if (_params.ignore_if_all_nans && allNans) {
    return;
  }

  if (_params.ignore_if_any_nans && anyNans) {
    return;
  }

  fprintf(out, "%s\n", ostr.c_str());
  _lineCount++;

#endif

}

