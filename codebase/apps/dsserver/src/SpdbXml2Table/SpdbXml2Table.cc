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
// SpdbXml2Table.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2011
//
///////////////////////////////////////////////////////////////
//
// SpdbXml2Table reads XML entries from an SPDB data base,
// and based on the specified parameters in the file,
// it reformats these into a text table.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include "SpdbXml2Table.hh"

using namespace std;

// Constructor

SpdbXml2Table::SpdbXml2Table(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "SpdbXml2Table";
  
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
  
}

// destructor

SpdbXml2Table::~SpdbXml2Table()

{

}

//////////////////////////////////////////////////
// Run

int SpdbXml2Table::Run ()
{

  _lineCount = 0;

  DsSpdb spdb;

  if (spdb.getInterval(_params.input_url,
                       _startTime.utime(),
                       _endTime.utime(),
                       _params.data_type,
                       _params.data_type_2)) {
    cerr << "ERROR - SpdbXml2Table::Run" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  // add comment lines is appropriate

  if (_params.add_commented_header) {
    _printComments(stdout);
  }

  // get chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    _printLine(stdout, chunks[ii]);
  }

  return 0;
  
}

//////////////////////////////////////////////////
// print comments at start

void SpdbXml2Table::_printComments(FILE *out)
{

  const char *com = _params.comment_character;

  // initial line has column headers

  fprintf(out, "%s count year month day hour min sec unix_time unix_day ", com);
  for (int ii = 0; ii < _params.xml_entries_n; ii++) {
    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    if (entry.specify_label) {
      fprintf(out, "%s ", entry.label);
    } else {
      fprintf(out, "%s ", entry.xml_tag_list);
    }
  } // ii
  fprintf(out, "\n");

  // then comment lines

  fprintf(out, "%s============================================\n", com);
  fprintf(out, "%s  Table produced by SpdbXml2Table\n", com);
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

}

//////////////////////////////////////////////////
// print a line of data

void SpdbXml2Table::_printLine(FILE *out,
                               const Spdb::chunk_t &chunk)

{

  if (chunk.data == NULL) {
    return;
  }

  char oline[10000];
  string ostr;
  

  // date and time

  DateTime vtime(chunk.valid_time);
  sprintf(oline, "%8d%s%.4d%s%.2d%s%.2d%s%.2d%s%.2d%s%.2d%s%ld%s%12.6f",
          _lineCount, _params.column_delimiter,
          vtime.getYear(), _params.column_delimiter,
          vtime.getMonth(), _params.column_delimiter,
          vtime.getDay(), _params.column_delimiter,
          vtime.getHour(), _params.column_delimiter,
          vtime.getMin(), _params.column_delimiter,
          vtime.getSec(), _params.column_delimiter,
          (long) vtime.utime(), _params.column_delimiter,
          (double) vtime.utime() / 86400.0);
  ostr += oline;
  
  // set the xml string from the chunk data

  string xml((const char *) chunk.data);
  
  // loop through XML entries

  bool allNans = true;
  
  for (int ii = 0; ii < _params.xml_entries_n; ii++) {
    
    // delimiter

    sprintf(oline, "%s", _params.column_delimiter);
    ostr += oline;

    const Params::xml_entry_t &entry = _params._xml_entries[ii];
    
    // get tag list
    
    vector<string> tags;
    TaStr::tokenize(entry.xml_tag_list, "<>", tags);
    if (tags.size() == 0) {
      if (entry.required) {
        return;
      }
      sprintf(oline, "nan");
      ostr += oline;
      continue;
    }
    
    string buf(xml);
    for (size_t jj = 0; jj < tags.size(); jj++) {
      string val;
      if (TaXml::readString(buf, tags[jj], val)) {
        if (entry.required) {
          return;
        }
        sprintf(oline, "nan");
        ostr += oline;
        continue;
      }
      if (jj == (tags.size() - 1)) {
        if (val.find("nan") != 0) {
          allNans = false;
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
            sprintf(oline, "1");
            ostr += oline;
          } else if (STRequal(val.c_str(), "FALSE")) {
            sprintf(oline, "0");
            ostr += oline;
          } else {
            sprintf(oline, "%s", val.c_str());
            ostr += oline;
          }
        } else {
          sprintf(oline, "%s", val.c_str());
          ostr += oline;
        }
      } else {
        buf = val;
      }
    }

  } // ii
  
  if (_params.ignore_if_all_nans && allNans) {
    return;
  }

  fprintf(out, "%s\n", ostr.c_str());
  _lineCount++;

}

