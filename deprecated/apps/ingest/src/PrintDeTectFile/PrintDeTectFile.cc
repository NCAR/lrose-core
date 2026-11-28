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
/**
 *
 * @file PrintDeTectFile.cc
 *
 * @class PrintDeTectFile
 *
 * PrintDeTectFile is the top level application class.
 *  
 * @date 7/19/2011
 *
 */

#include <assert.h>
#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <DeTect/ArchiveDirectory.hh>
#include <DeTect/ArchiveLabel.hh>
#include <DeTect/DataObject.hh>
#include <DeTect/DeTectFile.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "PrintDeTectFile.hh"

using namespace std;


// Global variables

PrintDeTectFile *PrintDeTectFile::_instance = (PrintDeTectFile *)NULL;


/*********************************************************************
 * Constructor
 */

PrintDeTectFile::PrintDeTectFile(int argc, char **argv)
{
  static const string method_name = "PrintDeTectFile::PrintDeTectFile()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (PrintDeTectFile *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
}


/*********************************************************************
 * Destructor
 */

PrintDeTectFile::~PrintDeTectFile()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

PrintDeTectFile *PrintDeTectFile::Inst(int argc, char **argv)
{
  if (_instance == (PrintDeTectFile *)NULL)
    new PrintDeTectFile(argc, argv);
  
  return(_instance);
}

PrintDeTectFile *PrintDeTectFile::Inst()
{
  assert(_instance != (PrintDeTectFile *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool PrintDeTectFile::init()
{
  static const string method_name = "PrintDeTectFile::init()";
  
  return true;
}
  


/*********************************************************************
 * run()
 */

void PrintDeTectFile::run()
{
  static const string method_name = "PrintDeTectFile::run()";
  
  _processData(_args->getFileName());
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData()
 */

bool PrintDeTectFile::_processData(const string &file_path)
{
  static const string method_name = "PrintDeTectFile::_processData()";
  
  cout << "*** Processing file: " << file_path << endl << endl;
  
  // Open the file

  DeTectFile input_file(file_path);
  
  if (!input_file.openFile())
    return false;

  // Print the beginning archive label

  ArchiveLabel archive_label;
  
  if (!input_file.readArchiveLabel(archive_label))
    return false;
  
  archive_label.print(cout, "", false);
  
  // Print the DataObjects

  size_t num_data_objects = 0;
  DateTime start_time = _args->getStartTime();
  DateTime end_time = _args->getEndTime();
  
  while (!input_file.isEndOfData())
  {
    // Get the next data object from the file and process it

    DataObject data_object;
    
    if (!input_file.readNextDataObject(data_object))
      return false;

    if (start_time != DateTime::NEVER &&
	data_object.getStartTime() < start_time)
      continue;
    
    if (end_time != DateTime::NEVER &&
	data_object.getStartTime() > end_time)
      break;
    
    data_object.print(cout, "", _args->isPrintAngles(), _args->isPrintData());
    
    ++num_data_objects;
    
  } /* endwhile - !input_file.isEndOfData() */
  
  // Print the archive directory information at the end of the file

  if (end_time == DateTime::NEVER)
  {
    cout << "*** File contains " << num_data_objects << " data objects" << endl;
  
    ArchiveDirectory archive_dir;
  
    if (!input_file.readArchiveDirectory(archive_dir))
      return false;

    archive_dir.print(cout, "");
  }
  
  return true;
}
