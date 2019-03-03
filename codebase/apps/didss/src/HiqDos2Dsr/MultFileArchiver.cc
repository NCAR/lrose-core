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
/*********************************************************************
 * MultFileArchiver: Class of objects that write given datasets to multiple
 *                   files.  An hour's worth of data is written to each file.
 *                   The files are stored under the given data directory in
 *                   the following paths:
 *
 *                           <data dir>/YYYYMMDD/HH0000.hiq
 *
 *                   where:
 *
 *                        YYYYMMDD is the year, month day of the ending time
 *                                of the data in the files in this directory
 *                        HH0000.hiq contains the beginning hour of the data
 *                                in this file.  The minute and second values
 *                                in this file name will always be 0.
 *
 *                    Note that the current system time (in GMT) is used for
 *                    file naming and the decision of when to begin a new file
 *                    so that this class doesn't have to know anything about
 *                    the underlying data being used.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/os_config.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>

#include "MultFileArchiver.hh"
using namespace std;


MultFileArchiver::MultFileArchiver(const bool debug) :
  FileArchiver(debug),
  _directoryPath(""),
  _currentFilePtr(0),
  _currentFileStartTime(DateTime::NEVER)
{
}

MultFileArchiver::~MultFileArchiver() 
{
  if (_currentFilePtr != 0)
    fclose(_currentFilePtr);
}


/*********************************************************************
 * init() - Initialize the archiver
 */

bool MultFileArchiver::init(const string directory_path)
{
  static const string method_name = "MultFileArchiver::init()";

  _archiverInitialized = false;

  // Save the directory path for use in creating file paths

  _directoryPath = directory_path;

  _archiverInitialized = true;

  return true;
}


/*********************************************************************
 * _updateOutputFilePtr() - Check to see if we need to open a new output file
 *                          and, if so, close the old output file and open the
 *                          new one.
 */

void MultFileArchiver::_updateOutputFilePtr(void)
{
  DateTime current_time(time(0));

  // Open a new file if it doesn't look like we've openned one previously

  if (_currentFileStartTime == DateTime::NEVER ||
      _currentFilePtr == 0)
  {
    _currentFileStartTime = current_time;
    _currentFileStartTime.setTime(current_time.getHour(), 0, 0);

    // Construct the new file name

    _openOutputFile();

    return;
  }

  // Open a new file if we've passed into the next hour

  if (current_time.getYear()  != _currentFileStartTime.getYear() ||
      current_time.getMonth() != _currentFileStartTime.getMonth() ||
      current_time.getDay()   != _currentFileStartTime.getDay() ||
      current_time.getHour()  != _currentFileStartTime.getHour())
  {
    _currentFileStartTime = current_time;
    _currentFileStartTime.setTime(current_time.getHour(), 0, 0);

    // Construct the new file name

    _openOutputFile();

    return;
  }
}


/*********************************************************************
 * _openOutputFile() - Close any open output files and open a new output
 *                     file based on the given file start time.
 */

void MultFileArchiver::_openOutputFile(void)
{
  static const string method_name = "MultFileArchiver::_openOutputFile()";

  // Close the old file if there is one

  if (_currentFilePtr != 0)
  {
    fclose(_currentFilePtr);
    _currentFilePtr = 0;
  }

  // Construct the output directory name and make sure it exists

  char pathname[MAX_PATH_LEN];
  sprintf(pathname, "%s/%04d%02d%02d",
	  _directoryPath.c_str(),
	  _currentFileStartTime.getYear(),
	  _currentFileStartTime.getMonth(),
	  _currentFileStartTime.getDay());

  if (ta_makedir_recurse(pathname) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating archive output directory: " << pathname << endl;

    return;
  }

  // Construct the new file name

  char filename[MAX_PATH_LEN];
  sprintf(filename, "%s/%02d0000.hiq",
	  pathname,
	  _currentFileStartTime.getHour());

  // Open the new file

  _currentFilePtr = fopen(filename, "w");
}
