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
 *  $Id: Amdar2Spdb.hh,v 1.5 2016/03/07 01:22:59 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Amdar2Spdb
//
// Author:	G. M. Cunning
//
// Date:	Sat Mar 17 13:52 2012
//
// Description: This class creates AMDAR SPDB messages for 
// simulation and testing purposes.
//
//

#ifndef    AMDAR_2_SPDB_H
#define    AMDAR_2_SPDB_H

// C++ include files
#include <ctime>
#include <string>
#include <vector>

// System/RAP include files
#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <dataport/port_types.h>

/////////////////////////
// Forward declarations
class Args;
class DsInputPath;
class DsSpdb;
class Input;
class Amdar;
class TextDecoder;
class BufrDecoder;
class Params;

/////////////////////////
// Class declaration
class Amdar2Spdb {
  
public:

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar2Spdb::instance
  //
  // Description:	this method creates instance of Amdar2Spdb object: 
  //			singleton instance invocations
  //
  // Returns:		returns pointer to self
  //
  // Notes:		this method implements the singleton pattern
  //
  //
  static Amdar2Spdb *instance(int argc, char **argv);
  static Amdar2Spdb *instance();

  // destructor  
  ~Amdar2Spdb();

  // getErrStr -- returns error message
  std::string getErrStr() const { return _errStr; }  // run 
  
  // isOK -- check on class
  bool isOK() const { return _isOK; }
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar2Spdb::run
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

  std::time_t _fileTime;
  DateTime _refTime;

  // Decoders
  TextDecoder* _textDecoder;
  BufrDecoder* _bufrDecoder;
  

  // Singleton instance pointer
  static Amdar2Spdb *_instance; 

  // hide the constructors and copy consturctor
  Amdar2Spdb();
  Amdar2Spdb(int argc, char **argv);
  Amdar2Spdb(const Amdar2Spdb &);
  Amdar2Spdb  &operator=(const Amdar2Spdb &);

  // int _addObservation(TaiwanAwos* awos_site);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar2Spdb::_initialize
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
  // Method Name:	Amdar2Spdb::_processFile
  //
  // Description:	performs the observation processing
  //
  // Returns:		returns 0 on success
  //
  // Notes:		
  //
  int _processFile(const string& file_path);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar2Spdb::_addPut
  //
  // Description:	adds chunks to the ascii and xml spdb files
  //
  // Returns:		none
  //
  // Notes:	
  //
  void _addPut(const std::vector<Amdar*>& amdars, std::vector<DsSpdb*>& ascii_spdb, 
	       std::vector<DsSpdb*>& xml_spdb);

  void _clearErrStr() { _errStr = ""; }

};


# endif   // AMDAR_2_SPDB_H
