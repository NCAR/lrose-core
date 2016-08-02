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
//   $Date: 2016/03/06 23:53:41 $
//   $Id: Reader.hh,v 1.2 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.2 $
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

#ifndef Reader_hh
#define Reader_hh

#include <vector>

#include "Archiver.hh"

using namespace std;


class Reader
{

public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructor
   */

  Reader(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~Reader();


  /////////////////////
  // Utility methods //
  /////////////////////

  /*********************************************************************
   * readBuffer() - Read the next buffer from the source.
   *
   * Returns the number of bytes read from the source.
   */

  int readBuffer(char *buffer, const int buffer_size);
   

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addArchiver() - Add an archiver to the archiver list.
   *
   * NOTE: AFtehr this method is called, the HiqReader object takes
   * control of the ARchiver pointer so this pointer shouldn't be
   * used or deleted by the calling object.
   */

  void addArchiver(Archiver *archiver)
  {
    _archivers.push_back(archiver);
  }


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  vector< Archiver* > _archivers;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _readBytes() - Read the next group of bytes from the source.
   *
   * Returns the number of bytes read from the source.
   */

  virtual int _readBytes(char *buffer, const int buffer_size) = 0;


};

#endif

   
