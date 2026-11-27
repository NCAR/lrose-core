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
/************************************************************************
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
 ************************************************************************/

#ifndef MultFileArchiver_hh
#define MultFileArchiver_hh

#include <cstdio>
#include <string>

#include <toolsa/DateTime.hh>

#include "FileArchiver.hh"

using namespace std;


class MultFileArchiver : public FileArchiver
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  MultFileArchiver(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~MultFileArchiver();


  /*********************************************************************
   * init() - Initialize the archiver
   */

  bool init(const string directory_path);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _directoryPath;

  FILE *_currentFilePtr;
  DateTime _currentFileStartTime;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getFilePtr() - Get the pointer to the file to use for archiving.
   */

  virtual FILE *_getFilePtr(void)
  {
    _updateOutputFilePtr();
    return _currentFilePtr;
  }


  /*********************************************************************
   * _updateOutputFilePtr() - Check to see if we need to open a new output file
   *                          and, if so, close the old output file and open the
   *                          new one.
   */

  void _updateOutputFilePtr(void);


  /*********************************************************************
   * _openOutputFile() - Close any open output files and open a new output
   *                     file based on the given file start time.
   */

  void _openOutputFile(void);

};

#endif
