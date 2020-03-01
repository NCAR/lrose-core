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
////////////////////////////////////////////////
//
// Mdvx_write.hh
//
// Write functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

// clear all write requests, set defaults
void clearWrite();

// write using extended paths
//
// If false, file path is:
//   topdir/yyyymmdd/hhmmss.mdv, or
//   topdir/yyyymmdd/g_hhmmss/f_ssssssss.mdv
//
// If true, file path is:
//   topdir/yyyymmdd/yyyymmdd_hhmmss.mdv, or
//   topdir/yyyymmdd/g_hhmmss/yyyymmdd_ghhmmss_fssssssss.mdv

void setWriteUsingExtendedPath();
void clearWriteUsingExtendedPath();

// write adding a subdir for the year
//
// If true, file path is:
//   topdir/yyyy/yyyymmdd....
//
// If false, file path is:
//   topdir/yyyymmdd....
//

void setWriteAddYearSubdir();
void clearWriteAddYearSubdir();

// write as forecast? forces forecast-style write
//
// If false, file path is computed as:
//   topdir/yyyymmdd/hhmmss.mdv
//
// If true, file path is computed as:
//   topdir/yyyymmdd/g_hhmmss/f_ssssssss.mdv, where g_hhmmss is generate
//   time and ssssssss is forecast lead time in secs.

void setWriteAsForecast();
void clearWriteAsForecast();

// if forecast, write as forecast? not forced
// Same as writeAsForecast, except it only writes as forecast if
// data_collection_type is FORECAST or EXTRAPOLATED

void setIfForecastWriteAsForecast();
void clearIfForecastWriteAsForecast();

// set write format
// default is classic

void setWriteFormat(mdv_format_t format);
void clearWriteFormat();

// write _latest_data_info file?
// If true, _latest_data_info file is written in top_dir

void setWriteLdataInfo();
void clearWriteLdataInfo();

// Get the size of an MDV file that would be written
// using 32-bit headers

si64 getWriteLen32() const;

//////////////////////////////////////////////////////
// Write to directory
//
// File path is computed - see setWriteAsForecast().
// _latest_data_info file is written as appropriate - 
//    see setWriteLdataInfo().
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

virtual int writeToDir(const string &output_dir);

/////////////////////////////////////////////////////////
// Write to path
//
// File is  written to specified path.
// Note: no _latest_data_info file is written.
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

virtual int writeToPath(const string &output_path);

/////////////////////////////////////////////////////////
// Write to buffer
// Write Mdvx object to a buffer as if written to file.

void writeToBuffer(MemBuf &buf) const;

/////////////////////////////////////////////////////////
// Write to buffer - 64-bit headers
// Write Mdvx object to a buffer as if written to file.

void writeToBuffer64(MemBuf &buf) const;

/////////////////////////////////////////////////////////
// Write to buffer - 32-bit headers
// Write Mdvx object to a buffer as if written to file.

void writeToBuffer32(MemBuf &buf) const;

///////////////////////////////////////////////////////
// Write to path using the buffer routine.
//
// This is intended for testing only.
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int writeUsingBuf(const string &output_path) const;

//////////////////////
// print write options

void printWriteOptions(ostream &out);


#endif

    
