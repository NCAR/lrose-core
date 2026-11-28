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
 *  $Id: AccessA2Mdv.hh,v 1.7 2017/11/08 03:02:20 cunning Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	AccessA2Mdv
//
// Author:	G. M. Cunning
//
// Date:	Fri Jul 23 21:00:36 2004
//
// Description: 
//
//
#ifndef    ACCESSA2MDV_H
#define    ACCESSA2MDV_H

// C++ include files
#include <string>
#include <vector>

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <netcdf.hh>

// Local include files

using namespace std;

/////////////////////////
// Forward declarations
class Args;
class Params;
class Navigation;
class InputStrategy;

/////////////////////////
// Class declaration
class AccessA2Mdv {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // instance -- create the Singleton
  static AccessA2Mdv *instance(int argc, char **argv);
  static AccessA2Mdv *instance();

  // getErrStr -- returns error message
  const string& getErrStr() const { return _errStr; }

  // isOK -- check on class
  const bool isOK() const { return _isOK; }

  // run 
  int run();

  // destructor
  ~AccessA2Mdv();

protected:

  
private:

  /////////////////////
  // private members //
  /////////////////////
  bool _isOK;
  string _progName;
  mutable string _errStr;
  Args *_args;
  Params *_params;

  InputStrategy *_inputStrategy;

  Navigation *_navigation;

  DsMdvx _outMdvx;
  time_t _dateTime;
  int forecastHour;

  // Singleton instance pointer
  static AccessA2Mdv *_instance;

  static const string _className;
  static const float MISSING_DATA_VALUE;
  static const float TINY_VALUE; 

  /////////////////////
  // private methods //
  /////////////////////

  // constructor -- hide since class is singleton
  AccessA2Mdv(int argc, char **argv);
  AccessA2Mdv(AccessA2Mdv&);
  AccessA2Mdv();

  void _clearErrStr() { _errStr = ""; }
  bool _checkFile(NcFile &ncf);
  void _getObservationTime(const string& file_path);
  void _getObservationTime(NcFile &ncf);
  bool _processFile(const string& file_path);
  bool _printContents(const string& file_path);
  void _setMasterHeader(const string& path, NcFile &ncf);
  bool _setupInput();
  int round(float x);
};

# endif   // ACCESSA2MDV_H
