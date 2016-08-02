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
/////////////////////////////////////////////////////////////
// AlertMetaFile.hh
//
// Class representing the ASCII file containing the alert net meta data.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2000
//
///////////////////////////////////////////////////////////////

#ifndef AlertMetaFile_H
#define AlertMetaFile_H

#include <iostream>
#include <cstdio>
#include <string>

#include <hydro/AlertMeta.hh>
using namespace std;



class AlertMetaFile
{
  
public:

  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  // constructors

  AlertMetaFile(const bool debug_flag = false);

  // destructor
  
  ~AlertMetaFile();


  ////////////////////
  // Access methods //
  ////////////////////

  // Retrieve the file time for the file.  Returns -1 is there isn't
  // a current input file.

  inline time_t getFileTime(void)
  {
    if (!_openFile())
      return -1;
    
    return _fileTime;
  }
  

  // Set the input file path.  This method will close the old file,
  // if there is one.

  void setFile(const string &input_path,
	       const time_t file_time = -1)
  {
    _closeFile();
    
    _inputFilePath = input_path;
    _fileTime = file_time;
  }


  // Get the next gauge from the current input file.  Note that you
  // MUST call setFile() before calling this method.
  //
  // If successful, returns a pointer to a newly created AlertMeta object
  // representing the next gauge in the file.  This pointer must be deleted
  // by the client.
  //
  // If not successful, returns 0.  Use getErrStr() to get the

  AlertMeta *getNextGauge(void);
  

  //////////////////////////
  // Error access methods //
  //////////////////////////

  inline bool isError(void) const
  {
    return _isError;
  }
  

  inline string &getErrStr(void) const
  {
    return _errStr;
  }
  

protected:
  
private:

  //////////////////////
  // Private typedefs //
  //////////////////////

  // Define the tokens expected on an input line

  typedef enum
  {
    PROV_ID_TOKEN,
    AFOS_ID_TOKEN,
    NAME_TOKEN,
    ELEV_TOKEN,
    LAT_TOKEN,
    LON_TOKEN,
    LOCAL_TZ_TOKEN,
    LOC_DESCR_TOKEN,
    STATION_TYPE_TOKEN,
    NUM_INST_TOKEN,
    NUM_LEV_TOKEN,
    MAINT_SCHED_TOKEN,
    SITE_DESCR_TOKEN,
    NUM_TOKENS
  } file_line_tokens_t;
  
  static const string FILE_DELIMITER;
  

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int MAX_TOKEN_LEN;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  

  // Input file information

  string _inputFilePath;
  
  FILE *_inputFile;
  
  time_t _fileTime;
  
  // Error information

  mutable string _errStr;
  mutable bool _isError;
  
  // Objects for parsing the tokens on an input line

  char **_tokens;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Close the current input file.

  void _closeFile(void);
  

  // Open the current input file.  Simply returns if the current input
  // file is already open.
  //
  // Returns true if the input file was successfully opened, false
  // otherwise.

  bool _openFile(void);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("AlertMetaFile");
  }
  
};

#endif
