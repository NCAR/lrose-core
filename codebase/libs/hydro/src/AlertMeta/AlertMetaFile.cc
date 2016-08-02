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
// AlertMetaFile.cc
//
// Class representing the ASCII file containing the alert net meta data.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <hydro/AlertMetaFile.hh>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;


// Define global constants


const int AlertMetaFile::MAX_TOKEN_LEN = 1024;

const string AlertMetaFile::FILE_DELIMITER = "|";


/*********************************************************************
 * Constructors
 */

AlertMetaFile::AlertMetaFile(const bool debug_flag) :
  _debugFlag(debug_flag),
  _inputFilePath(""),
  _inputFile(0),
  _fileTime(-1)
{
  // Allocate space for the token parsing object

  _tokens = new char* [NUM_TOKENS];
  for (int i = 0; i < (int)NUM_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
}


/*********************************************************************
 * Destructor
 */

AlertMetaFile::~AlertMetaFile()
{
  // Close any open files

  _closeFile();
  
  // Reclaim space for the token parsing object

  for (int i = 0; i < (int)NUM_TOKENS; ++i)
    delete [] _tokens[i];
  
  delete [] _tokens;
}


/*********************************************************************
 * getNextGauge() - Get the next gauge from the current input file.
 *
 * If successful, returns a pointer to a newly created AlertMeta object
 * representing the next gauge in the file.  This pointer must be deleted
 * by the client.
 *
 * If not successful, returns 0.  Use getErrStr() to get the
 */

AlertMeta *AlertMetaFile::getNextGauge(void)
{
  const string method_name = "getNextGauge()";
  
  // Initialize the error information

  _errStr = string("ERROR: ") + _className() + "::" + method_name + "\n";
  _isError = false;
  
  // Make sure the input file is open

  if (!_openFile())
  {
    _isError = true;
    
    return 0;
  }
  
  // Find the next true input line in the file

  const int INPUT_LINE_LEN = 1024;
  char input_line[INPUT_LINE_LEN];
  bool input_line_found = false;
  
  while (fgets(input_line, INPUT_LINE_LEN, _inputFile) != 0)
  {
    if (input_line[0] != '#')
    {
      input_line_found = true;
      break;
    }
  }
  
  // See if there were more input lines in the file.  If not, return 0
  // but don't set the error codes since this isn't an error, just the
  // end of the file.

  if (!input_line_found)
    return 0;
  
  // Parse the tokens in the input line

  int num_tokens;
  
  if ((num_tokens = STRparse_delim(input_line, _tokens, INPUT_LINE_LEN,
				   FILE_DELIMITER.c_str(),
				   NUM_TOKENS, MAX_TOKEN_LEN)) != NUM_TOKENS)
  {
    _errStr += string("Cannot parse meta data line: ") + input_line;
    _errStr += "Wrong number of tokens on line\n";
    TaStr::AddInt(_errStr, "Expected ", NUM_TOKENS);
    TaStr::AddInt(_errStr, " tokens, got ", num_tokens);
    _errStr += " tokens\n";
    
    return 0;
  }
  
  // Remove the surrounding blanks from the string tokens

  STRblnk(_tokens[AFOS_ID_TOKEN]);
  STRblnk(_tokens[NAME_TOKEN]);
  STRblnk(_tokens[LOCAL_TZ_TOKEN]);
  STRblnk(_tokens[LOC_DESCR_TOKEN]);
  STRblnk(_tokens[STATION_TYPE_TOKEN]);
  STRblnk(_tokens[MAINT_SCHED_TOKEN]);
  STRblnk(_tokens[SITE_DESCR_TOKEN]);

  // Create the AlertMeta object to return

  return new AlertMeta(atoi(_tokens[PROV_ID_TOKEN]),
		       _tokens[AFOS_ID_TOKEN],
		       _tokens[NAME_TOKEN],
		       atof(_tokens[ELEV_TOKEN]),
		       atof(_tokens[LAT_TOKEN]),
		       atof(_tokens[LON_TOKEN]),
		       atoi(_tokens[NUM_INST_TOKEN]),
		       atoi(_tokens[NUM_LEV_TOKEN]),
		       _tokens[LOCAL_TZ_TOKEN],
		       _tokens[LOC_DESCR_TOKEN],
		       _tokens[STATION_TYPE_TOKEN],
		       _tokens[MAINT_SCHED_TOKEN],
		       _tokens[SITE_DESCR_TOKEN],
		       _debugFlag);
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/*********************************************************************
 * _closeFile() - Close the current input file.
 */

void AlertMetaFile::_closeFile(void)
{
  if (_inputFile != 0)
  {
    fclose(_inputFile);
    _inputFile = 0;
  }

  _fileTime = -1;
}


/*********************************************************************
 * _openFile() - Open the current input file.  Simply returns if the
 *               current input file is already open.
 *
 * Returns true if the input file was successfully opened, false
 * otherwise.
 */

bool AlertMetaFile::_openFile(void)
{
  const string method_name = "_openFile()";
  
  // See if the file is already open

  if (_inputFile != 0)
    return true;
  
  // Make sure we have an input file path already

  if (_inputFilePath.size() == 0)
  {
    _errStr += "Input file path not yet specified\n";
    _isError = true;
    
    return false;
  }
  
  // See if we can open the file

  if ((_inputFile = fopen(_inputFilePath.c_str(), "r")) == 0)
  {
    _errStr += "Error opening meta-data file <" + _inputFilePath +
      "> for reading\n";
    _isError = true;
    
    return false;
  }
  
  // Now retrieve the file time from the file

  if (_fileTime <= 0)
    _fileTime = time(0);
  
  // If we got here, the file was opened successfully

  return true;
}
