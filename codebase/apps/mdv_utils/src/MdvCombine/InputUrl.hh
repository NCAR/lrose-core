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
 *  $Id: InputUrl.hh,v 1.6 2016/03/04 02:22:10 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header: InputUrl
// 
// Author: G M Cunning
// 
// Date:	Wed Jan 17 13:02:02 2001
// 
// Description:	this class is a wrapper around the DsMdvxTimes class.
// 
// 
// 
// 


# ifndef    INPUT_URL_H
# define    INPUT_URL_H

// C++ include files
#include <string>
using namespace std;

/////////////////////////
// Forward declarations
class Params;
class Args;
class Mdvx;
class MdvxField;
class DsMdvx;
class DsMdvxTimes;

class InputUrl {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  InputUrl(const string &url,
	   const int field_number, const string &field_name,
	   const int time_trigger_interval,
	   const bool debug = false);

  // destructor
  virtual ~InputUrl();

  // isOK -- check on class
  bool isOK() const { return _isOk; }

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }

  // read in the relevant file
  void getNext(const time_t& request_time);

  // update the start and end times
  void updateTimes(time_t *start_time_p, time_t *end_time_p);

  // was latest read a success?
  bool readSuccess() const { return _readSuccess; }

  // file path
  string path() const { return _path; }

  // returns a copy of the master header
  Mdvx::master_header_t* getMasterHeader();

  // returns a copy of the master header
  MdvxField* getField(void);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////

  bool _debug;
  
  string _url; // data URL/directory
  long _fieldNumber;
  string _fieldName;
  
  long _timeTriggerInterval;
  
  bool _isOk;
  string _errStr;
  static const string _className;

  bool _readSuccess; // set true if last read was a success

  DsMdvx _handle; 
  Mdvx::master_header_t _masterHeader;

  string _path; // actual path used

  time_t _time; // actual time for data in file


  /////////////////////
  // private methods //
  /////////////////////


};


# endif     /* INPUT_URL_H */
