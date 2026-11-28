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
/*
 *  $Id: Metreport2Spdb.hh,v 1.5 2016/03/07 01:23:02 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Metreport2Spdb
//
// Author:	G. M. Cunning
//
// Date:	June 17 2014
//
// Description: 
//
//

#ifndef    METREPORT_2_SPDB_H
#define    METREPORT_2_SPDB_H

// C++ include files
#include <ctime>
#include <string>
#include <vector>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
#include <Spdb/StationLoc.hh>

/////////////////////////
// Forward declarations
class Args;
class DsInputPath;
class DsSpdb;
class Input;
class Params;
class StationLoc;

/////////////////////////
// Class declaration
class Metreport2Spdb {
  
public:

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::instance
  //
  // Description:	this method creates instance of Metreport2Spdb object: 
  //			singleton instance invocations
  //
  // Returns:		returns pointer to self
  //
  // Notes:		this method implements the singleton pattern
  //
  //
  static Metreport2Spdb *instance(int argc, char **argv);
  static Metreport2Spdb *instance();

  // destructor  
  ~Metreport2Spdb();

  // getErrStr -- returns error message
  std::string getErrStr() const { return _errStr; }  // run 
  
  // isOK -- check on class
  bool isOK() const { return _isOK; }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::run
  //
  // Description:	runs the object
  //
  // Returns:		returns 0
  //
  // Notes:	
  //
  //
  int run();

protected:
  
private:

  bool _isOK;
  std::string _progName;
  std::string _errStr;
  Args *_args;
  Params *_params;
  static const std::string _className;

  // Input objects

  DsInputPath *_inputPath;
  Input *_input;

  // report info
  std::string _name;
  std::time_t _validTime;
  std::time_t _refTime;

  DateTime _fileTime;

  // station location
  
  StationLoc *_stationLoc;

  // Singleton instance pointer
  static Metreport2Spdb *_instance; 

  // hide the constructors and copy consturctor
  Metreport2Spdb();
  Metreport2Spdb(int argc, char **argv);
  Metreport2Spdb(const Metreport2Spdb &);
  Metreport2Spdb  &operator=(const Metreport2Spdb &);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_initialize
  //
  // Description:	Initializes the class.
  //
  // Returns:		none
  //
  // Notes:		use isOK method to test success
  //
  void _initialize(int argc, char **argv);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_processFile
  //
  // Description:	performs the observation processing
  //
  // Returns:		returns 0 on success
  //
  // Notes:		
  //
  int _processFile(const std::string& file_path);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_parse
  //
  // Description:	performs parsing of bulletin to identify the site
  //			name, valid time, and expire time
  //
  // Returns:		returns 0 on success
  //
  // Notes:		
  //
  int _decode(const std::string& report);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_setTime
  //
  // Description:	sets the valid time using report time and file
  //			file time
  //
  // Returns:		returns valid time as time_t
  //
  // Notes:		
  //
  std::time_t _setTime(const std::string& rtime);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_parse
  //
  // Description:	performs parsing of bulletin to identify the site
  //			name, valid time, and expire time
  //
  // Returns:		returns 0 on success
  //
  // Notes:		
  //
  bool _acceptStation();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Metreport2Spdb::_addPut
  //
  // Description:	adds chunks to the spdb files
  //
  // Returns:		none
  //
  // Notes:	
  //
  void _addPut(const std::string& report, DsSpdb& out_spdb);

  void _clearErrStr() { _errStr = ""; }

};


# endif   // METREPORT_2_SPDB_H
