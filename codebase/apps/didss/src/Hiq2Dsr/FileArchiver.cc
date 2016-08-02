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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: FileArchiver.cc,v 1.2 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FileArchiver: Class of objects that write given datasets to a 
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>

#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>

#include "FileArchiver.hh"

using namespace std;


FileArchiver::FileArchiver(const bool debug) :
  Archiver(debug),
  _archiverInitialized(false)
{
}

FileArchiver::~FileArchiver() 
{
}


/*********************************************************************
 * archiveData() - Write the given data buffer to the archive.
 */

void FileArchiver::archiveData(const char *input_buffer,
			       const int input_buffer_len)
{
  static const string method_name = "FileArchiver::archiveData()";

  // Make sure the archiver was initialized

  if (!_archiverInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "PROGRAMMING ERROR -- FileArchiver derived object not initialized" << endl;
    cerr << "Message not archived..." << endl;

    return;
  }

  // Let the outside world know what we're doing

  PMU_auto_register("Writing raw beam to archive file");

  if (_debug)
    cerr << "Writing buffer to archive file" << endl;

  // Get a pointer to the output file.  This method is virtual and must
  // be overriden by derived classes.

  FILE *file_ptr;

  if ((file_ptr = _getFilePtr()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting file pointer for output" << endl;
    cerr << "Nothing will be written to the archive file" << endl;

    return;
  }

  // First write the buffer length to the file

  ui16 output_buffer_len = BE_from_si16(input_buffer_len);

  if (fwrite((void *)&output_buffer_len, sizeof(ui16), 1, file_ptr) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not write buffer length to archive file" << endl;
      
    return;
  }

  // Now write the buffer to the file

  if (fwrite((void *)input_buffer, sizeof(char), (size_t)input_buffer_len,
	     file_ptr) != (size_t)input_buffer_len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not write beam data to archive file" << endl;
      
    return;
  }
}

