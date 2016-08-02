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
// $Id: HiqMsg.hh,v 1.6 2016/03/06 23:53:40 dixon Exp $
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
    EOL_BEAM_MSG,
    EOL_RADAR_MSG,
    ARC_BEAM_MSG,
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
