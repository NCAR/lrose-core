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
// DataFileNames.hh - class to get dates and times from data file names.
// Also gets elapsed times since access and modification, and
// if the file has been zipped. 
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1998
//
/////////////////////////////////////////////////////////////

#ifndef DATAFILENAMES_HH
#define DATAFILENAMES_HH

#include <string>
#include <time.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>
using namespace std;

class DataFileNames {
  
public:



  // public methods.

  void GetFileFacts(const char *FileName, const char *CompressedExt,
		    const char *MatchExt, const char *FilePattern=NULL);

  void GetFileFacts(const string FileName, const string CompressedExt,
		    const string MatchExt, const string FilePattern="" );

  // Set the public variables for a file.

  void PrintFileFacts(FILE *out = NULL) const;
  // Print out the public variables.
  // By default print to stdout

  // Find out how full the disk is.

  int PercentFullDisk(const string FileName) const;

  int PercentFullDisk(const char *FileName) const;

  // Get the data time information from the given file path.
  //
  // The following formats are supported.
  //    (* indicates any sequence of non-digits)
  //    (? indicates a single non-digit)
  //
  //     */yyyymmdd/g_hhmmss/f_llllllll.ext
  //     */yyyymmdd/hhmmss.ext
  //     */*yyyymmdd?hhmmss*
  //     */*yyyymmddhhmmss*
  //     */*yyyymmddhhmm*
  //     */*yyyymmddhh.tmhhmm (mm5 forecast)
  //     */*yyyymmddhh*
  //     */*yyyyjjjhhmm*
  //     */*yyyyjjjhh*
  //     */*yyyymmdd?hhmm*
  //     */*yyyymmdd?hh*
  //     */*yyyymmdd*
  //     */*yyjjj*
  //
  // Returns 0 on success, -1 on error.
  // On success, sets data_time.
  
  static int getDataTime(const string& file_path, time_t &data_time,
			 bool &date_only, bool gen_time = false);

  static int getDataTime(const string& file_path,
                         const string& file_pattern, time_t &data_time,
			 bool &date_only, bool gen_time = false);

  // The public variables
  bool   Exists;      // Does the file exist?

  bool   DateOnly;    // If true then the filename does not
  //                     have the time, only the date,
  //                     ie. YYYYMMDD.data and the time is
  //                     arbitrarily set to noon on this day.

  // The file must exist for the following to be valid.
  bool IsSoftLinkDir;  // File is a soft linked Directory
  bool IsPathRelative; // Link Is path relative - May be a dangerous
                       //  circular link if  IsSoftLinkDir is TRUE
  time_t ModTime;    // Since modfication.
  time_t AccessTime; // Since access.
  bool Directory;   // true if the file is actually a directory.
  bool Regular;     // true if it's a regular old file.
  unsigned long FileSize; // File size, bytes.

  // The following may be valid if the file exists or not.

  date_time_t NameDate; // Date from the name, if any.
  bool NameDateValid; // Can we get a date from this name?
  bool Compressed; // Is the file compressed?
  bool ExtMatches; // True if the extension mates that specified by MatchExt.
  // This class has no private members. It's pretty simple.



};

#endif

