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
//   $Date: 2016/03/04 02:22:15 $
//   $Id: ArchiveFileRetriever.cc,v 1.4 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ArchiveFileRetriever.cc: Class for retrieving archive DsMdvx files.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <sys/time.h>

#include <Mdv/DsMdvx.hh>
#include <toolsa/umisc.h>

#include "ArchiveFileRetriever.hh"
using namespace std;


/*********************************************************************
 * Constructor
 *
 * Note that this constructor exits the program if there is an error
 * compiling the time list to be processed.
 */

ArchiveFileRetriever::ArchiveFileRetriever(const string& input_url,
					   const time_t start_time,
					   const time_t end_time,
					   const bool debug_flag) :
  FileRetriever(input_url, debug_flag)
{
  const string routine_name = "Constructor";
  
  // Compile the file time list

  DsMdvx time_compiler;
    
  time_compiler.setTimeListModeValid(input_url,
				     start_time, end_time);
    
  if (time_compiler.compileTimeList() != 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error compiling file time list" << endl;
      
    exit(-1);
  }

  _archiveTimeList = time_compiler.getTimeList();
}


/*********************************************************************
 * Destructor
 */

ArchiveFileRetriever::~ArchiveFileRetriever(void)
{
  // Do nothing
}


/*********************************************************************
 * next() - Method for retrieving the next file to process.
 */

DsMdvx *ArchiveFileRetriever::next(void)
{
  const string routine_name = "next()";
  
  // See if there are any more files to process

  if (_archiveTimeList.empty())
    return 0;
      
  // Get the next data file time

  time_t file_time = _archiveTimeList.front();
    
  _archiveTimeList.erase(_archiveTimeList.begin());

  // Read the new data file

  DsMdvx *new_mdv_file;
  
  if ((new_mdv_file = _readFile(file_time)) == 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error reading file for time " << utimstr(file_time) << endl;
  }

  return new_mdv_file;
}


DsMdvx *ArchiveFileRetriever::_readFile(const time_t search_time)
{
   const string routine_name = "_readFile()";

   if (_debugFlag)
     cout << "Reading file for time " << utimstr(search_time) << endl;

   // Create the file object

   DsMdvx *mdv_file = new DsMdvx();
   
   // Read in the data

   mdv_file->clearRead();
   mdv_file->setReadTime(Mdvx::READ_CLOSEST,
			 _inputUrl, 0, search_time);
//   mdv_file->clearReadFields();
//   mdv_file->addReadField(_params->field_num);
   mdv_file->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
   mdv_file->setReadCompressionType(Mdvx::COMPRESSION_NONE);
   mdv_file->setReadScalingType(Mdvx::SCALING_NONE);
   
   if (_debugFlag)
     mdv_file->printReadRequest(cout);
   
   if (mdv_file->readVolume() != 0)
   {
     delete mdv_file;
     
     return 0;
   }
   
   return mdv_file;
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

