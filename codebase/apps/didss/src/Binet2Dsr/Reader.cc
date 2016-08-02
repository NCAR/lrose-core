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
//   $Id: Reader.cc,v 1.5 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Reader: Base class for objects used to read raw beam data from any
 *         source.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include "Reader.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

Reader::Reader(const bool debug) :
  _debug(debug)
{
}


/*********************************************************************
 * Destructor
 */

Reader::~Reader() 
{
  // Delete the archiver pointers.

  vector< Archiver* >::iterator archiver_iter;
  for (archiver_iter = _archivers.begin(); archiver_iter != _archivers.end();
       ++archiver_iter)
    delete *archiver_iter;
}


/*********************************************************************
 * readBuffer() - Read the next buffer from the source.
 *
 * Returns the number of bytes read from the source.
 */

int Reader::readBuffer(char *buffer, const int buffer_size)
{
  static const string method_name = "Reader::readBuffer()";

  // First read the buffer from the actual source.  Here we are calling
  // a method in the derived class.

  int buf_len = _readBytes(buffer, buffer_size);

  if (buf_len <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Number of bytes read by reader: " << buf_len << endl;

    return buf_len;
  }

  // Archive the raw data wherever requested

  vector< Archiver* >::iterator archive_iter;

  for (archive_iter = _archivers.begin(); archive_iter != _archivers.end();
       ++archive_iter)
  {
    Archiver *archiver = *archive_iter;

    archiver->archiveData(buffer, buf_len);
  } /* endfor - archive_iter */
  
  return buf_len;
}
