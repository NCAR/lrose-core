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
//   $Id: EdgeBeamMsg.cc,v 1.3 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeBeamMsg : An EDGE message with beam data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <string.h>

#include "EdgeBeamMsg.hh"
using namespace std;


// Define constants


/*********************************************************************
 * Constructor
 */

EdgeBeamMsg::EdgeBeamMsg(const EdgeMsgSupport::moment_type_t moment_type)
{
  // Set the constant values in the header

  _msgHeader.setMomentType(moment_type);
}


/*********************************************************************
 * Destructor
 */

EdgeBeamMsg::~EdgeBeamMsg() 
{
  // Do nothing
}


/*********************************************************************
 * setData() - Set the beam data to the given buffer.
 */

void EdgeBeamMsg::setData(const double azimuth, const double elevation,
			  const void *beam_data, const unsigned int num_gates)
{
  // First copy the data into the uncompressed buffer.

  _uncompressedBuffer.reset();
  _uncompressedBuffer.load(beam_data, num_gates);
  _msgHeader.setUncompressedLen(num_gates);
  
  // Now compress the data.  This method will also update the compressed length
  // in the message header.

  _compressData();

  // Finally, set the azimuth and elevation in the header

  _msgHeader.setAzimuth(azimuth);
  _msgHeader.setElevation(elevation);
}


/*********************************************************************
 * writeMsgToBuffer() - Write the message data to the given buffer.
 *                      The buffer must be allocated by the calling
 *                      method.  The needed buffer size can be found
 *                      using getMsgSize();
 *
 * Returns true if the message was successfully written to the buffer,
 * false otherwise.
 */

bool EdgeBeamMsg::writeMsgToBuffer(void *msg_buffer)
{
  // Initialize the buffer

  memset(msg_buffer, 0, _msgHeader.getHeaderSize() + _compressedBuffer.getLen());

  // Get a pointer into the buffer for writing

  char *buffer_ptr = (char *)msg_buffer;
  
  // First write the message header to the buffer.

  _msgHeader.addHeaderToBuffer(msg_buffer);
  
  // Now add the message information to the buffer.  We will write
  // each record separately updating the buffer pointer as we go.

  buffer_ptr += _msgHeader.getHeaderSize();
  
  memcpy((void *)buffer_ptr, _compressedBuffer.getPtr(),
	 _compressedBuffer.getLen());
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _compressData() - Compress the current beam data in the object.
 */

void EdgeBeamMsg::_compressData()
{
  // If there isn't currently any uncompressed data, make sure the
  // compressed data buffer is also empty.

  if (_uncompressedBuffer.getLen() == 0)
  {
    _compressedBuffer.reset();
    return;
  }
  
  // Currently, we don't support any compressions methods so just copy
  // the uncompressed buffer into the compressed buffer.  At some point,
  // this should be changed to use a compression strategy object.

  _compressedBuffer.reset();
  _compressedBuffer.load(_uncompressedBuffer.getPtr(),
			 _uncompressedBuffer.getLen());
  
  _msgHeader.setCompressedLen(_compressedBuffer.getLen());
}
