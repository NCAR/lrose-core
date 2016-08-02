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
//   $Id: RapFileReader.hh,v 1.2 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RapFileReader: Class for objects used to read beam data from a RAP
 *                HiQ file.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef RapFileReader_hh
#define RapFileReader_hh

#include <cstdio>
#include <string>
#include <vector>

#include "Reader.hh"

using namespace std;


class RapFileReader : public Reader
{

public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructor
   */

  RapFileReader(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  ~RapFileReader();


  /*********************************************************************
   * init() - Initialize the reader.
   *
   * Returns true on success, false on failure.
   */

  bool init(const vector<string> file_list,
	    const int msgs_per_sec);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  vector<string> _fileList;
  
  FILE *_currentFilePtr;
  
  int _msgSleepMsecs;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _prepareInputFile() - Check the current file pointer.  If it is NULL
   *                       or we are at the end-of-file open the next
   *                       input file.
   */

  void _prepareInputFile();
  

  /*********************************************************************
   * _readBytes() - Read the next group of bytes from the source.
   *
   * Returns the number of bytes read from the source.
   */

  virtual int _readBytes(char *buffer, const int buffer_size);
   

};

#endif
