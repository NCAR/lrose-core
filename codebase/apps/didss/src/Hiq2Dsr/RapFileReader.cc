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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: RapFileReader.cc,v 1.3 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RapFileReader: Class for objects used to read beam data from a RAP
 *                HiQ file.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <toolsa/uusleep.h>
#include <iostream>
#include <cstdlib>
#include "RapFileReader.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

RapFileReader::RapFileReader(const bool debug) :
  Reader(debug),
  _currentFilePtr(0)
{
}


/*********************************************************************
 * Destructor
 */

RapFileReader::~RapFileReader() 
{
  // Close any open files

  if (_currentFilePtr != 0)
    fclose(_currentFilePtr);
}


/*********************************************************************
 * init() - Initialize the reader.
 *
 * Returns true on success, false on failure.
 */

bool RapFileReader::init(const vector<string> file_list,
			 const int msgs_per_sec)
{
  _fileList = file_list;
  
  _msgSleepMsecs =
    (int)((1.0 / (float)msgs_per_sec * 1000.0) + 0.5);
  
  return true;
}


/*********************************************************************
 * _prepareInputFile() - Check the current file pointer.  If it is NULL
 *                       or we are at the end-of-file open the next
 *                       input file.
 */

void RapFileReader::_prepareInputFile()
{
  static const string method_name = "RapFileReader::_prepareInputFile()";
  
  while (_currentFilePtr == 0 ||
	 feof(_currentFilePtr))
  {
    // Close any open files

    if (_currentFilePtr != 0)
      fclose(_currentFilePtr);
    _currentFilePtr = 0;
    
    // Make sure there are more files in the list

    if (_fileList.size() <= 0)
      exit(0);
    
    // Open the next input file and remove it from the file list
    // so we don't process any input files twice.

    if (_debug)
      cerr << "*** Opening file <" << _fileList[0] << "> for input" << endl;
    
    if ((_currentFilePtr = fopen(_fileList[0].c_str(), "r")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening input file: " << _fileList[0] << endl;
    }
    
    _fileList.erase(_fileList.begin(), _fileList.begin()+1);
  }
}


/*********************************************************************
 * _readBytes() - Read the next group of bytes from the source.
 *
 * Returns the number of bytes read from the source.
 */

int RapFileReader::_readBytes(char *buffer, const int buffer_size)
{
  static const string method_name = "RapFileReader::readMsg()";
  
  // Sleep to simulate real-time

  umsleep(_msgSleepMsecs);
  
  // Make sure we have an open input file

  _prepareInputFile();
  
  if (_currentFilePtr == 0)
    return 0;
  
  // Read the buffer length from the file

  ui16 input_buffer_len;
  
  if (fread(&input_buffer_len, sizeof(input_buffer_len), 1,
	    _currentFilePtr) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading buffer length from input file" << endl;
    cerr << "Skipping rest of input file..." << endl;
    
    fclose(_currentFilePtr);
    _currentFilePtr = 0;
    
    return 0;
  }
  
  input_buffer_len = BE_to_si16(input_buffer_len);
  
  // Make sure there is enough space in the provided buffer for
  // the data

  if (input_buffer_len > buffer_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Provided buffer too small for next message in file" << endl;
    cerr << "Skipping message" << endl;
    
    char *temp_input_buffer = new char[input_buffer_len];
    fread(temp_input_buffer, sizeof(char), input_buffer_len,
	  _currentFilePtr);
    delete[] temp_input_buffer;
    
    return 0;
  }
  
  // Read the input buffer from the file

  if (fread(buffer, sizeof(char), input_buffer_len, _currentFilePtr)
      != input_buffer_len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading next message from input file" << endl;
    cerr << "Message has " << input_buffer_len << " bytes." << endl;
    
    return 0;
  }
  
  return input_buffer_len;
}
