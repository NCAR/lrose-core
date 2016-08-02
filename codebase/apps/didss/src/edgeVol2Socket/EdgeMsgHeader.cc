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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: EdgeMsgHeader.cc,v 1.4 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeMsgHeader : Class used for manipulating the header portion of
 *                 an EDGE message.  All EDGE messages contain the
 *                 same header.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <string>
#include <iostream>

#include "EdgeMsgHeader.hh"
using namespace std;


// Define constants

const int EdgeMsgHeader::HEADER_SIZE = 40;


/*********************************************************************
 * Constructor
 */

EdgeMsgHeader::EdgeMsgHeader() :
  _azimuth(0.0),
  _elevation(0.0),
  _checkSum(0),
  _uncompressedLen(0),
  _compressedLen(0),
  _momentType(EdgeMsgSupport::EDGE_STATUS_MOMENT),
  _compressionType(EdgeMsgSupport::COMPRESSION_NONE),
  _rollingCount(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

EdgeMsgHeader::~EdgeMsgHeader() 
{
  // Do nothing
}


/*********************************************************************
 * addHeaderToBuffer() - Puts the current header in the given buffer.
 *                       The calling method must have allocated enough
 *                       space for the header in the buffer.  The header
 *                       size is given by the getHeaderSize() method.
 *
 * Returns true if successful, false otherwise.
 */

bool EdgeMsgHeader::addHeaderToBuffer(void *msg_buffer) 
{
  static const string method_name = "EdgeMsgHeader::addHeaderToBuffer()";
  
  if (snprintf((char *)msg_buffer, HEADER_SIZE,
	       "%04x %04x %08x %4d %4d %04x %d %02x",
	       EdgeMsgSupport::degToBinary(_azimuth),
	       EdgeMsgSupport::degToBinary(_elevation),
	       _checkSum, _uncompressedLen, _compressedLen,
	       _momentType, _compressionType, _rollingCount) <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing header to given message buffer" << endl;
    
    return false;
  }
  
  ++_rollingCount;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
