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
//   $Id: SingleFileArchiver.cc,v 1.4 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SingleFileArchiver: Class of objects that write given datasets to a 
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>

#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>

#include "SingleFileArchiver.hh"

using namespace std;


SingleFileArchiver::SingleFileArchiver(const bool debug) :
  FileArchiver(debug),
  _filePtr(0)
{
}

SingleFileArchiver::~SingleFileArchiver() 
{
  if (_filePtr != 0)
    fclose(_filePtr);
}


/*********************************************************************
 * init() - Initialize the archiver
 */

bool SingleFileArchiver::init(const string file_path)
{
  static const string method_name = "SingleFileArchiver::init()";

  _archiverInitialized = false;

  // Make sure the output directory exists

  Path output_path(file_path);
  
  if (output_path.makeDirRecurse() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not create output directory for archive file: "
	 << file_path << endl;
    
    return false;
  }
    
  if ((_filePtr = 
       ta_fopen_uncompress((char *)file_path.c_str(), "a")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not open archive file: " << file_path << endl;
      
    return false;
  }

  _archiverInitialized = true;

  return true;
}
