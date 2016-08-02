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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:23:02 $
//   $Id: LtgWriter.cc,v 1.5 2016/03/07 01:23:02 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LtgWriter: Class for handling writing lightning strike data to the
 *            output file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <toolsa/file_io.h>

#include "LtgWriter.hh"

using namespace std;

const string LtgWriter::FILE_EXT = "ualf";


/*********************************************************************
 * Constructor
 */

LtgWriter::LtgWriter() :
  _outputDir("."),
  _secondsPerFile(1),
  _currFile(0),
  _currFileTime(DateTime::NEVER)
{
}


/*********************************************************************
 * Destructor
 */

LtgWriter::~LtgWriter()
{
  _closeFile();
}


/*********************************************************************
 * init() - Initialize the object.  This must be called before any other
 *          methods.
 *
 * Returns true on success, false on failure.
 */

bool LtgWriter::init(const string &output_dir,
		     const int seconds_per_file)
{
  static const string method_name = "LtgWriter::init()";
  
  // Save the values

  _outputDir = output_dir;
  _secondsPerFile = seconds_per_file;
  
  // Create the output directory

  if (ta_makedir_recurse(_outputDir.c_str()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << _outputDir << endl;

    return false;
  }
  
  // Set up the ldata info

  _ldataInfo.setDir(output_dir);
  _ldataInfo.setWriter("LnpIngest");
  _ldataInfo.setDataFileExt(FILE_EXT.c_str());
  _ldataInfo.setDataType("ascii");
  _ldataInfo.setIsFcast(false);
  
  return true;
}


/*********************************************************************
 * writeStrike() - Write the given strike to the appropriate output
 *                 file.
 *
 * Returns true on success, false on failure.
 */

bool LtgWriter::writeStrike(const LtgStrike &strike)
{
  // Make sure we are using the appropriate output file

  if (!_openFile(strike.getTime()))
    return false;
  
  strike.writeUalf(_currFile);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getFirstFileTime() - This is called when we don't have a file currently
 *                       open.  So, we need to figure out the correct file
 *                       time to use by starting at the beginning of the
 *                       current hour and iterating through the specified
 *                       file duration.
 *
 * Returns the correct file time to use.
 */

DateTime LtgWriter::_getFirstFileTime(const DateTime &strike_time) const
{
  DateTime file_time(strike_time);
  
  file_time.setMin(0);
  file_time.setSec(0);
  
  while (strike_time >= file_time + _secondsPerFile)
    file_time += _secondsPerFile;
  
  return file_time;
}


/*********************************************************************
 * _getNextFileTime() - This is called when we have a file currently open,
 *                      but the current strike needs to go in a new file.
 *                      In this case we start at the current file time and
 *                      iterate through the specified file duration until
 *                      we get the appropriate file for the new strike.
 *
 * Returns the correct file time to use.
 */

DateTime LtgWriter::_getNextFileTime(const DateTime &strike_time) const
{
  DateTime file_time(_currFileTime);
  
  while (strike_time >= file_time + _secondsPerFile)
    file_time += _secondsPerFile;
  
  return file_time;
}


/*********************************************************************
 * _openFile() - Make sure that the currently open file is the appropriate
 *               one for the given time.
 *
 * Returns true on success, false on failure.
 */

bool LtgWriter::_openFile(const DateTime &strike_time)
{
  static const string method_name = "LtgWriter::_openFile()";
  
  // See if we need to open a new file

  if (_currFileTime != DateTime::NEVER &&
      strike_time < _currFileTime + _secondsPerFile)
    return true;
  
  // Close the current file

  _closeFile();
  
  // Figure out the appropriate name for the new file and open it

  DateTime file_time;
  
  if (_currFileTime == DateTime::NEVER)
    file_time = _getFirstFileTime(strike_time);
  else
    file_time = _getNextFileTime(strike_time);

  char filename[BUFSIZ];
  
  sprintf(filename, "%04d%02d%02d_%02d%02d%02d.%s",
	  file_time.getYear(), file_time.getMonth(), file_time.getDay(),
	  file_time.getHour(), file_time.getMin(), file_time.getSec(),
	  FILE_EXT.c_str());
  
  _currFilename = filename;
  
  string file_path = _outputDir + "/" + _currFilename;
  
  cerr << "Output file path: " << file_path << endl;
  
  if ((_currFile = fopen(file_path.c_str(), "a")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file: " << file_path << endl;
    perror(file_path.c_str());
    
    return false;
  }
  
  _currFileTime = file_time;
  
  return true;
}
