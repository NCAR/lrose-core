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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/06 23:53:42 $
 *   $Id: EdgeBeamMsg.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeBeamMsg : An EDGE message with beam data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeBeamMsg_HH
#define EdgeBeamMsg_HH


#include <toolsa/MemBuf.hh>

#include "EdgeMsg.hh"
#include "EdgeMsgSupport.hh"
using namespace std;

class EdgeBeamMsg : public EdgeMsg
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  EdgeBeamMsg(const EdgeMsgSupport::moment_type_t moment_type);


  /*********************************************************************
   * Destructor
   */

  virtual ~EdgeBeamMsg();


  /*********************************************************************
   * setData() - Set the beam data to the given buffer.
   */

  virtual void setData(const double azimuth, const double elevation,
		       const void *beam_data, const unsigned int num_gates);


  /*********************************************************************
   * getMsgSize() - Returns the number of bytes needed to store the whole
   *                message for transmission.
   */

  virtual int getMsgSize()
  {
    return _msgHeader.getHeaderSize() + _msgHeader.getCompressedLen();
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

  virtual bool writeMsgToBuffer(void *msg_buffer);


protected:

  MemBuf _uncompressedBuffer;
  MemBuf _compressedBuffer;
  

  /*********************************************************************
   * _compressData() - Compress the current beam data in the object.
   */

  void _compressData();
  
};

#endif

   
