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
 *  $Id: TextDecoder.hh,v 1.8 2016/03/07 01:22:59 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	TextDecoder
// 
// Author:	G. M. cunning
// 
// Date:	Mon Mar 19 10:35 2012
// 
// Description:	Decoder subclass that handles text messages following 
//		FM 42 -XI Ext. See WMO AMDAR reference manual.
// 
// 


# ifndef    TEXT_DECODER_H
# define    TEXT_DECODER_H

// C++ include files

// System/RAP include files

// Local include files

#include "Decoder.hh"

class TextDecoder : public Decoder {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  TextDecoder();
  TextDecoder(const Params *params);
  TextDecoder(const TextDecoder &);

  // destructor
  virtual ~TextDecoder();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::checkFormat
  //
  // Description:	checks to see if message is FM42 (ASCII) or FM94
  //			(BUFR)  
  //
  // Returns:		a value from msg_type_t
  //
  // Notes:	
  //
  msg_type_t checkFormat(const string &hdr, const string &msg); 

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::process
  //
  // Description:	decode the message 
  //
  // Returns:		returns 0 for succes, and -1 otherwise
  //
  // Notes:		Allocates memory for the Amdar objects in amdars
  //			vector. Not responsible for destruction.
  //
  int process(const std::string& amdar_str, std::vector<Amdar*>& amdars);

  TextDecoder &operator=(const TextDecoder &from);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

  // set the WMO and type IDs
  void _setID();

private:

  /////////////////////
  // private members //
  /////////////////////

  static const std::string _className;

  static const std::string SECTION_3_ID;

  static const string MISSING;
  static const char DELIM = ' ';

  static const size_t _rhPosition = 7;

  static const size_t MIN_BULLETIN_LEN = 40;

  /////////////////////
  // private methods //
  /////////////////////

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::_parse
  //
  // Description:	parses out bulletin information from string
  //
  // Returns:		none
  //
  // Notes:	
  //
  void _parse(const string& bulletin, Amdar::ascii_std_amdar_bulletin_t& ascii_amdar);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::_setPhaseOfFlight
  //
  // Description:	normalizes the tag for the phase of flight
  //
  // Returns:		string containing one of the recognized tags
  //
  // Notes:		leaving as a pass through right now.
  //
  string _setPhaseOfFlight(const string& text);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::_setTime
  //
  // Description:	sets time of bulletin
  //
  // Returns:		bulletin time
  //
  // Notes:	
  //
  time_t _setTime(const string& btime);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	TextDecoder::_copy
  //
  // Description:	performs the deep copy
  //
  // Returns:		none
  //
  // Notes:	
  //
  void _copy(const TextDecoder& from);

};

# endif     /* TEXT_DECODER_H */
