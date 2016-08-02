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
//////////////////////////////////////////////////////////
// $Id: HiqMsg.hh,v 1.2 2016/03/06 23:53:40 dixon Exp $
//
// Edge Message class
/////////////////////////////////////////////////////////

#ifndef HiqMsg_hh
#define HiqMsg_hh


#include <iostream>

using namespace std;


class HiqMsg
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    BEAM_MSG,
    RADAR_MSG,
    UNKNOWN_MSG
  } msg_type_t;


  /*********************************************************************
   * Constructors
   */

  HiqMsg(const bool debug = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~HiqMsg();


  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getMsgType() - Return the type of this message object.
   */

  virtual msg_type_t getMsgType() const = 0;


  /*********************************************************************
   * isHeader() - Returns true if this message starts with a header,
   *              false otherwise.
   */

  static inline bool isHeader(char *msg_ptr)
  {
    // Check for beam message

    if (msg_ptr[0] == 'D' && msg_ptr[1] == 'W' &&
	msg_ptr[2] == 'E' && msg_ptr[3] == 'L')
      return true;
    
    // Check for radar message

    if (msg_ptr[0] == 'R' && msg_ptr[1] == 'H' &&
	msg_ptr[2] == 'D' && msg_ptr[3] == 'R')
      return true;
    
    return false;
  }


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  virtual void print(ostream &stream) const = 0;

  virtual void printSummary(ostream &stream) const = 0;


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

};

#endif
