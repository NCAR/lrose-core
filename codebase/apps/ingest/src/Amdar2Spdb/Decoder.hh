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
 *  $Id: Decoder.hh,v 1.4 2016/03/07 01:22:59 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	Decoder
// 
// Author:	G. M. cunning
// 
// Date:	Mon Mar 19 10:00 2012
// 
// Description:	Base class for decoding AMDAR messages.
// 
// 


# ifndef    DECODER_H
# define    DECODER_H

// C++ include files
#include <string>
#include <vector>

// System/RAP include files
#include <rapformats/Amdar.hh>

// Local include files
#include "Params.hh"
 

class Decoder {
  
public:

  typedef enum {
    MSG_TYPE_ASCII = 0,
    MSG_TYPE_BUFR = 1,
    MSG_TYPE_UNKNOWN = 2 
  } msg_type_t;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  Decoder();
  Decoder(const Params *params);
  Decoder(const Decoder& from );

  // destructor
  virtual ~Decoder();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::checkFormat
  //
  // Description:	checks to see if message is FM42 (ASCII) or FM94
  //			(BUFR)  
  //
  // Returns:		a value from msg_type_t
  //
  // Notes:	
  //
  virtual msg_type_t checkFormat(const string &hdr, const string &msg) = 0; 

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::process
  //
  // Description:	decode the message 
  //
  // Returns:		returns 0 for succes, and -1 otherwise
  //
  // Notes:	
  //
  virtual int process(const std::string& amdar_str, std::vector<Amdar*>& amdars) = 0;

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::getWmoId
  //
  // Description:	gets the _wmoId 
  //
  // Returns:		returns reference to _wmoId
  //
  // Notes:	
  //
  const std::string &getWmoId() const { return _wmoId; }

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::getTypeId
  //
  // Description:	gets the _typeId 
  //
  // Returns:		returns reference to _typeId
  //
  // Notes:	
  //
  const std::string &getTypeId() const { return _typeId; }

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Decoder::setFileTime
  //
  // Description:	sets the file time. It is used to complete the
  //			issut time, because bulletin only contains day,
  //			hour and minute.
  //
  // Returns:		none
  //
  // Notes:	
  //
  void setFileTime(std::time_t file_time) { _fileTime = file_time; }

  Decoder &operator=(const Decoder &from);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////

  const Params *_params;
 
  // file time
  std::time_t _fileTime;

  // the WMO ID 
  std::string _wmoId;

  // the type ID 
  std::string _typeId;

  // set the WMO and type IDs
  virtual void _setID() = 0;

  ///////////////////////
  // protected methods //
  ///////////////////////


  
private:

};


# endif     /* DECODER_H */
