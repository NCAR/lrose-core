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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:01:42 $
//   $Id: AsciiOutput.cc,v 1.2 2016/03/04 02:01:42 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AsciiOutput: Class for creating ASCII output.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <fstream>
#include <iostream>
#include <unistd.h>

#include "AsciiOutput.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

AsciiOutput::AsciiOutput(const string &output_dir,
			 const string &output_ext,
			 const string &delimiter,
			 const vector< string > &statistic_names,
			 const bool include_header,
			 const bool debug_flag) :
  Output(output_dir, output_ext, debug_flag),
  _delimiter(delimiter),
  _includeHeader(include_header),
  _headerWritten(false),
  _statNames(statistic_names),
  _outputBuffer("")
{
}

AsciiOutput::AsciiOutput(const string &accum_path,
			 const string &delimiter,
			 const vector< string > &statistic_names,
			 const bool include_header,
			 const bool debug_flag) :
  Output(accum_path, debug_flag),
  _delimiter(delimiter),
  _includeHeader(include_header),
  _headerWritten(false),
  _statNames(statistic_names),
  _outputBuffer("")
{
}

  
/*********************************************************************
 * Destructor
 */

AsciiOutput::~AsciiOutput()
{
}


/*********************************************************************
 * addOutput() - Add the given information to the output buffer.
 *
 * Returns true on success, false on failure.
 */

bool AsciiOutput::addOutput(const DateTime &data_time,
			    const int simple_track_num,
			    const int complex_track_num,
			    const double vert_level,
			    const vector< double > &statistics)
{
  char string_buffer[BUFSIZ];
  
  sprintf(string_buffer, "%d%s%d%s%d%s%d%s%d%s%d%s%d%s%d%s%f",
	  data_time.getYear(), _delimiter.c_str(),
	  data_time.getMonth(), _delimiter.c_str(),
	  data_time.getDay(), _delimiter.c_str(),
	  data_time.getHour(), _delimiter.c_str(),
	  data_time.getMin(), _delimiter.c_str(),
	  data_time.getSec(), _delimiter.c_str(),
	  simple_track_num, _delimiter.c_str(),
	  complex_track_num, _delimiter.c_str(),
	  vert_level);
  _outputBuffer += string_buffer;
  
  vector< double >::const_iterator stat_val;
  
  for (stat_val = statistics.begin(); stat_val != statistics.end(); ++stat_val)
  {
    sprintf(string_buffer, "%s%f",
	    _delimiter.c_str(), *stat_val);
    _outputBuffer += string_buffer;
  }
  
  _outputBuffer += "\n";
  
  return true;
}


/*********************************************************************
 * writeOutput() - Write the information to the output file.
 *
 * Returns true on success, false on failure.
 */

bool AsciiOutput::writeOutput(const DateTime &file_time)
{
  static const string method_name = "AsciiOutput::writeOutput()";
  
  // Determine the output file path

  Path output_path;
  
  if (_accumulateData)
  {
    output_path.setPath(_accumPath);
  }
  else
  {
    output_path = _generateOutputPath(file_time);
    unlink(output_path.getPath().c_str());
  }
  
  if (_debug)
    cerr << "---> Writing output to path: " << output_path.getPath() << endl;
  
  // Open the output file

  if (output_path.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating directory path for output file: "
	 << output_path.getPath() << endl;
    
    return false;
  }
  
  // Open the output stream

  ofstream output_stream(output_path.getPath().c_str(), ios::app);
  
  // Write the header, if requested

  if (_includeHeader && !_headerWritten)
  {
    output_stream << "Year" << _delimiter << "Month" << _delimiter
		  << "Day" << _delimiter << "Hour" << _delimiter
		  << "Minute" << _delimiter << "Second" << _delimiter
		  << "SimpleTrackNum" << _delimiter
		  << "ComplexTrackNum" << _delimiter
		  << "VertLevel";
    
    vector< string >::const_iterator stat_name;
    
    for (stat_name = _statNames.begin(); stat_name != _statNames.end();
	 ++stat_name)
      output_stream << _delimiter << *stat_name;
    
    output_stream << endl;

    _headerWritten = true;
    
  }
  
  // Write the output buffer to the file

  output_stream << _outputBuffer;
  
  // Close the output file

  output_stream.close();
  
  // Clear the output buffer

  _outputBuffer = "";
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
