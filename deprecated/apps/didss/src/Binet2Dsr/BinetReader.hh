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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:39 $
//   $Id: BinetReader.hh,v 1.3 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BinetReader: Class for objects used to read Binet beam data.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef BinetReader_hh
#define BinetReader_hh

#include <toolsa/MemBuf.hh>

#include "BinetMsg.hh"
#include "Reader.hh"

using namespace std;


class BinetReader
{

public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructor
   */

  BinetReader(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  ~BinetReader();


  /*********************************************************************
   * init() - Initialize the reader.
   *
   * Returns true on success, false on failure.
   *
   * NOTE: After this method is called, the BinetReader object takes
   * control of the Reader pointer so this pointer shouldn't be
   * used or deleted by the calling object.
   */

  bool init(Reader *reader);


  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * getNextMsg() - Get the next Binet message.
   *
   * Returns a pointer to the latest message read on success; 0 otherwise.
   *
   * NOTE: The calling object takes control of the returned msg object and
   * is expected to delete this object when it is no longer needed.
   */

  BinetMsg *getNextMsg();
   

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int    MAX_BUF_SIZE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  Reader *_reader;

  MemBuf _messageBuffer;
  char *_inputBuffer;
  int _inputBufferLen;

};

#endif
