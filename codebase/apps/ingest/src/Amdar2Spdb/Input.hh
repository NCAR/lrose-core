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
 *  $Id: Input.hh,v 1.3 2016/03/07 01:22:59 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	Input
// 
// Author:	G. M. cunning
// 
// Date:	Sun Mar 18 12:30 2012
// 
// Description:	Class for reading AMDAR messages in files.
// 
// 


# ifndef    INPUT_H
# define    INPUT_H

// C++ include files
#include <string>
#include <iostream>
#include <fstream>

// System/RAP include files

// Local include files
#include "Params.hh"

class Input {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  Input(const Params *params);

  // destructor
  virtual ~Input();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::open
  //
  // Description:	opens the input file
  //
  // Returns:		0 for succes and -1 for an error
  //
  // Notes:
  //
  int open(const std::string& file_path);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::close
  //
  // Description:	closes the input file
  //
  // Returns:	
  //
  // Notes:
  //
  void close();
  
  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::readNext
  //
  // Description:	reads next message from file
  //
  // Returns:		returns 0 on success, -1 on failure (no more AMDARs)
  //
  // Notes:
  //
  int readNext(std::string& msg_str);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::setSOH
  //
  // Description:	sets the start-of-header (SOH) string 
  //
  // Returns:		none
  //
  // Notes:		string usually contains control characters
  //
  void setSOH(char soh) { _soh = soh;}

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::setEOH
  //
  // Description:	sets the end-of-header (EOH) string 
  //
  // Returns:		none
  //
  // Notes:		string usually contains control characters
  //
  void setEOH(char eoh) { _eoh = eoh;}

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::addTypeId
  //
  // Description:	add a message type ID to list of IDs to look for in
  //			messages
  //
  // Returns:		none
  //
  // Notes:		
  //
  void addTypeId(const std::string id) { _typeIds.push_back(id);}

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::addWmoId
  //
  // Description:	add a WMO header ID to list of IDs to look for in
  //			messages
  //
  // Returns:		none
  //
  // Notes:		
  //
  void addWmoId(const std::string id) { _wmoIds.push_back(id);}

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::getWmoHeader
  //
  // Description:	gets the _wmoHeader
  //
  // Returns:		returns reference to _wmoHeader
  //
  // Notes:	
  //
  const std::string &getWmoHeader() const { return _wmoHeader; }

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

  static const std::string _className;
 
  const Params *_params;
 
  // input file stream
  std::ifstream _inStream;

  // name of input file
  std::string _filePath;

  // start of header
  char _soh;

  // end of header
  char _eoh;

  // list of WMO header IDs to look for in the header line (.e.g UD for upper air)
  std::vector<std::string> _wmoIds;

  // list of message type IDs  to look for in messages
  std::vector<std::string> _typeIds;

  // message built from lines in the input file
  std::string _message;
 
  // WMO header line
  std::string  _wmoHeader;

  /// a flag to indicate that a message is being built
  bool _inBlock;

  /////////////////////
  // private methods //
  /////////////////////

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Input::_checkType
  //
  // Description:	check to see if _wmoHeader against list of desired
  //			IDs in _wmoIds
  //
  // Returns:		true if a match is made, otherwise false
  //
  // Notes:		
  //
  bool _checkType(const std::string &line, const std::vector< std::string >& type_list); 


};


# endif     /* INPUT_H */
