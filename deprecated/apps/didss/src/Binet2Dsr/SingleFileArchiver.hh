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
 *   $Date: 2016/03/06 23:53:39 $
 *   $Id: SingleFileArchiver.hh,v 1.4 2016/03/06 23:53:39 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SingleFileArchiver: Class of objects that write given datasets to a file.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SingleFileArchiver_hh
#define SingleFileArchiver_hh

#include <cstdio>
#include <string>

#include "FileArchiver.hh"

using namespace std;


class SingleFileArchiver : public FileArchiver
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  SingleFileArchiver(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~SingleFileArchiver();


  /*********************************************************************
   * init() - Initialize the archiver
   */

  bool init(const string file_path);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  FILE *_filePtr;

  ///////////////////////////////
  // Protected virtual methods //
  ///////////////////////////////

  /*********************************************************************
   * _getFilePtr() - Get the pointer to the file to use for archiving.
   */

  virtual FILE *_getFilePtr(void)
  {
    return _filePtr;
  }

};

#endif
