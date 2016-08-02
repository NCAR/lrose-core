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
 *   $Id: EdgeMsgHeader.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
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
 ************************************************************************/

#ifndef EdgeMsgHeader_HH
#define EdgeMsgHeader_HH


#include "EdgeMsgSupport.hh"
using namespace std;


class EdgeMsgHeader
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructors
   */

  EdgeMsgHeader();


  /*********************************************************************
   * Destructor
   */

  ~EdgeMsgHeader();


  ////////////////////////////
  // Message Buffer Methods //
  ////////////////////////////

  /*********************************************************************
   * getHeaderSize() - Returns the number of bytes in the message header.
   */

  inline int getHeaderSize()
  {
    return HEADER_SIZE;
  }
  

  /*********************************************************************
   * addHeaderToBuffer() - Puts the current header in the given buffer.
   *                       The calling method must have allocated enough
   *                       space for the header in the buffer.  The header
   *                       size is given by the getHeaderSize() method.
   *
   * Returns true if successful, false otherwise.
   */

  bool addHeaderToBuffer(void *msg_buffer);


  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * getAzimuth() - Retrieve the azimuth value from the header.  The azimuth
   *                value will be in degrees.
   */

  inline double getAzimuth() const
  {
    return _azimuth;
  }
  

  /*********************************************************************
   * setAzimuth() - Set the azimuth value in the header.  The given azimuth
   *                value should be in degrees.
   */

  void setAzimuth(const double azimuth)
  {
    _azimuth = azimuth;
  }
  

  /*********************************************************************
   * getElevation() - Retrieve the elevation value from the header.  The
   *                  elevation value will be in degrees.
   */

  inline double getElevation() const
  {
    return _elevation;
  }
  

  /*********************************************************************
   * setElevation() - Set the elevation value in the header.  The given
   *                  elevation value should be in degrees.
   */

  void setElevation(const double elevation)
  {
    _elevation = elevation;
  }
  

  /*********************************************************************
   * getCheckSum() - Retrieve the check sum value from the header.
   */

  inline unsigned int getCheckSum() const
  {
    return _checkSum;
  }
  

  /*********************************************************************
   * setCheckSum() - Set the check sum value in the header.
   */

  void setCheckSum(const unsigned int check_sum)
  {
    _checkSum = check_sum;
  }
  

  /*********************************************************************
   * getUncompressedLen() - Retrieve the uncompressed length value from the
   *                        header.  This value is the number of bytes
   *                        in the message (not including the header)
   *                        after the message has been uncompressed.
   */

  inline unsigned int getUncompressedLen() const
  {
    return _uncompressedLen;
  }
  

  /*********************************************************************
   * setUncompressedLen() - Set the uncompressed length value in the
   *                        header.  This value is the number of bytes
   *                        in the message (not including the header)
   *                        after the message has been uncompressed.
   */

  void setUncompressedLen(const unsigned int uncompressed_len)
  {
    _uncompressedLen = uncompressed_len;
  }
  

  /*********************************************************************
   * getCompressedLen() - Retrieve the compressed length value in the header.
   *                      This value is the number of bytes in the message
   *                      (not including the header) before the message has
   *                      been uncompressed (ie. the number of bytes in
   *                      the message as it is sent over a socket).
   */

  inline unsigned int getCompressedLen() const
  {
    return _compressedLen;
  }
  

  /*********************************************************************
   * setCompressedLen() - Set the compressed length value in the header.
   *                      This value is the number of bytes in the message
   *                      (not including the header) before the message has
   *                      been uncompressed (ie. the number of bytes in
   *                      the message as it is sent over a socket).
   */

  void setCompressedLen(const unsigned int compressed_len)
  {
    _compressedLen = compressed_len;
  }
  

  /*********************************************************************
   * getMomentType() - Retrieve the moment type value from the header.
   */

  inline EdgeMsgSupport::moment_type_t getMomentType() const
  {
    return _momentType;
  }
  

  /*********************************************************************
   * setMomentType() - Set the moment type value in the header.
   */

  void setMomentType(const EdgeMsgSupport::moment_type_t moment_type)
  {
    _momentType = moment_type;
  }
  

  /*********************************************************************
   * getCompressionType() - Retrieve the compression type value from the
   *                        header.
   */

  inline EdgeMsgSupport::compression_type_t getCompressionType() const
  {
    return _compressionType;
  }
  

  /*********************************************************************
   * setCompressionType() - Set the compression type value in the header.
   */

  void setCompressionType(const EdgeMsgSupport::compression_type_t compression_type)
  {
    _compressionType = compression_type;
  }
  

private:

  /////////////////////
  // Private members //
  /////////////////////

  static const int HEADER_SIZE;
  
  // Local members containing information for the header

  double _azimuth;
  double _elevation;
  unsigned int _checkSum;
  unsigned int _uncompressedLen;
  unsigned int _compressedLen;
  EdgeMsgSupport::moment_type_t _momentType;
  EdgeMsgSupport::compression_type_t _compressionType;
  unsigned int _rollingCount;              // Value cannot be set by user
  

  /////////////////////
  // Private methods //
  /////////////////////

};

#endif

   
