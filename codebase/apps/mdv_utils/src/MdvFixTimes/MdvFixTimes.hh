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
 *  $Id: MdvFixTimes.hh,v 1.3 2016/03/04 02:22:11 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	MdvFixTimes
//
// Author:	G. M. Cunning
//
// Date:	Fri Apr 22 16:10:07 2005
//
// Description: modifies master and field header times.
//
//

#ifndef    MDV_FIX_TIMES_H
#define    MDV_FIX_TIMES_H

// C++ include files
#include <string>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>

using namespace std;

/////////////////////////
// Forward declarations
class Params;
class Args;
class DsMdvxInput;

/////////////////////////
// Class declaration
class MdvFixTimes {
  
public:

  // singleton instance invocations
  static MdvFixTimes *instance(int argc, char **argv);
  static MdvFixTimes *instance();

  // destructor  
  ~MdvFixTimes();

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }  // run 
  
  // initialize -- initialize the object
  void initialize();

  // isOK -- check on class
  bool isOK() const { return _isOK; }
  
  // run -- Execution of program 
  int run();

protected:
  
private:

  bool _isOK;
  string _progName;
  string _errStr;
  Args *_args;
  Params *_params;

  DsMdvxInput *_input;

  static const string _className;

  // Singleton instance pointer
  static MdvFixTimes *_instance; 

  // hide the constructors and copy constructor
  MdvFixTimes();
  MdvFixTimes(int argc, char **argv);
  MdvFixTimes(const MdvFixTimes &);
  MdvFixTimes  &operator=(const MdvFixTimes &);

  void _clearErrStr() { _errStr = ""; }

};

# endif   // MDV_FIX_TIMES_H
