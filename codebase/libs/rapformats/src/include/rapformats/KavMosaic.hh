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

/*********************************************************************
 * KavMosaic: class manipulating a Kavouras mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef KavMosaic_H
#define KavMosaic_H


#include <string>

#include <rapformats/km.h>
#include <toolsa/MemBuf.hh>
using namespace std;


class KavMosaic
{
  
public:

  //////////////////
  // Constructors //
  //////////////////

  KavMosaic();
  

  ////////////////
  // Destructor //
  ////////////////
  
  virtual ~KavMosaic();


  //////////////////////////
  // Input/Output Methods //
  //////////////////////////

  /**********************************************************************
   * writeFile() - Write the Kavouras file to the given output directory.
   *
   * Returns true on success, false on failure.
   */

  bool writeFile(const string output_dir);
  

  ////////////////////
  // Access Methods //
  ////////////////////

  /**********************************************************************
   * setHeader() - Set the Kavouras file header.
   */

  virtual void setHeader(const KM_header_t header)
  {
    _header = header;
  }
  

protected:

  /////////////////////////
  // Protected Constants //
  /////////////////////////

  static const string _MONTH_ARRAY[12];
  

  ///////////////////////
  // Protected Members //
  ///////////////////////

  KM_header_t _header;
  char *_grid;
  

  ///////////////////////
  // Protected Methods //
  ///////////////////////

  /**********************************************************************
   * _writeGridToOutputBuffer() - Write the data grid to the output buffer.
   */
  
  void _writeGridToOutputBuffer();
  

  /**********************************************************************
   * _writeHeaderToOutputBuffer() - Write the header information to the
   *                                output buffer.
   */

  void _writeHeaderToOutputBuffer();
  

private:

  /////////////////////
  // Private Members //
  /////////////////////

  MemBuf _outputBuffer;
};

#endif

