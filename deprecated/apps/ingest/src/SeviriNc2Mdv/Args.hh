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
 *  $Id: Args.hh,v 1.3 2016/03/07 01:23:05 dixon Exp $
 *
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
//
// Class:	Args
//
// Author:	G. M. Cunning
//
// Date:	Tue Apr 17 15:13:02 2007
//
// Description: This class handles the command line arguments.
//		
//


#ifndef ARGS_H
#define ARGS_H

// C++ include files
#include <string>
#include <vector>
#include <iostream>


// System/RAP include files
#include <tdrp/tdrp.h>

using namespace std;

class Args {
  
public:

  // Flag indicating whether the current object status is okay.
  bool isOK;

  // TDRP overrides specified in the command line arguments.
  tdrp_override_t override;

  // constructor
  Args();

  // destructor
  ~Args();

  // parse out arguments
  void parse(int argc, char **argv, const string& prog_name);

  ////////////////////
  // Access methods //
  ////////////////////

  const time_t& getArchiveStartTime() const
  {
    return _archiveStartTime;
  }
  
  const time_t& getArchiveEndTime() const
  {
    return _archiveEndTime;
  }

  const vector<string>& getInputFileList() const
  {
    return _inputFileList;
  }
  
  const vector<string>& getInputDirList() const
  {
    return _inputDirList;
  }
  
  
protected:
  
private:

  static const string _className;

  // Archive start and end times
  time_t _archiveStartTime;
  time_t _archiveEndTime;

  vector<string> _inputFileList;
  vector<string> _inputDirList;


  // Disallow the copy constructor and assignment operator
  Args( const Args & );
  Args &operator=( const Args & );

  void _setOverride( const string& );
  void _setOverride( const string&, const char* );

  // Print the usage for this program.
  void _usage( const string &prog_name,
	       ostream& out);
  

};

#endif
