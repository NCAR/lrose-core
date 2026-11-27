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
/////////////////////////////////////////////////////////////
// ArchiveProcessor.hh: Class which controls a data directory in the
//               Archiver program.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2001
//
/////////////////////////////////////////////////////////////

#ifndef ArchiveProcessor_HH
#define ArchiveProcessor_HH

#include <string>

#include <didss/DataFileNames.hh>

#include "Params.hh"
using namespace std;


class ArchiveProcessor
{
  
public:

  // constructor
  
  ArchiveProcessor(const string &prog_name,
		   const Params::debug_t debug_level = Params::DEBUG_OFF);

  // Destructor

  ~ArchiveProcessor();

  // public data
  
  bool OK;

  // Process the directory
  
  bool processDir(const string &data_dir,
		  const string &data_rap_subpath,
		  Params *params,
		  const time_t start_time, const time_t end_time);

protected:
  
private:

//  // Check to see if the current hostname matches the expected one
//
//  bool _hostNamesMatch(const string expected_hostname) const;
//
//  // Check to see if the given directory name matches a RAP date
//  // directory.
//
//  static bool _dirMatchesDateFormat(const string dir_path);
//
  // Process the given file

  void _processFile(const string data_file_path,
		    const DataFileNames &data_file_info,
		    const time_t start_time,
		    const time_t end_time,
		    const string archive_dir,
		    const string filename,
		    const bool compress_file,
		    const char *compress_cmd);
  
  // Compress the given file

  void _compressFile(const string &file_path,
		     const char *compress_cmd) const;

  // Load the local parameters

  Params *_loadLocalParams(const string data_dir,
			   const Params &global_params);
  
  static void _addDelim(string &dir_path) ;
  
  string _progName;
  
  Params::debug_t _debugLevel;
  
};

#endif



