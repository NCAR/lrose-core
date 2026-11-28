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
 *  $Id: MdvThin.hh,v 1.8 2016/03/04 02:22:15 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	MdvThin
//
// Author:	G M Cunning
//
// Date:	Thu Mar  2 11:23:41 2000
//
// Description: This class thins out and synchronizes a 
//		data stream. The data stream is supportted 
//		through MDV files.
//

#ifndef    MDVTHIN_H
#define    MDVTHIN_H

// C++ include files
#include <string>
#include <ctime>
#include <vector>

// System/RAP include files
#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>
using namespace std;

// Local include files

/////////////////////////
// Forward declarations
class Args;
class Params;

/////////////////////////
// Class declaration
class MdvThin {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // instance -- create the singleton
  static MdvThin *instance(int argc, char **argv);
  static MdvThin *instance();

  // getErrStr -- returns error message
  const string& getErrStr() const { return _errStr; }

  // isOK -- check on class
  const bool isOK() const { return _isOK; }

  // run 
  int run();

  // destructor
  ~MdvThin();

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
  bool _isOK;
  string _progName;
  string _errStr;
  Args *_args;
  Params *_params;

  DsTrigger *_trigger;
  
  // Singleton instance pointer
  static MdvThin *_instance;  

  /////////////////////
  // private methods //
  /////////////////////

  // constructor -- hide, since class is singleton
  MdvThin(int argc, char **argv);
  MdvThin(MdvThin&);
  MdvThin();
  bool _processMessage(const DateTime &trigger_time);
  void _clearErrStr() { _errStr = ""; }
  void _updateTimes(Mdvx &mdvx, const DateTime &trigger_time) const;

};

# endif   // MDVTHIN_H
