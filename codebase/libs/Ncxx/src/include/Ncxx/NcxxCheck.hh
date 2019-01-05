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
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifndef NcxxCheckFunction
#define NcxxCheckFunction

/*! 
  Function checks error code and if necessary throws an exception.
  \param retCode Integer value returned by %netCDF C-routines.
  \param file    The name of the file from which this call originates.
  \param line    The line number in the file from which this call originates.
  \param context Optional extra context from the calling routine stack, to make
                 the exception message more meaningful
*/
void ncxxCheck(int retCode,
               std::string file, 
               int line,
               std::string context1 = "",
               std::string context2 = "",
               std::string context3 = "");

/*! 
  Function checks if the file (group) is in define mode.
  If not, it places it in the define mode.
  While this is automatically done by the underlying C API
  for netCDF-4 files, the netCDF-3 files still need this call.
*/
void ncxxCheckDefineMode(int ncid, std::string context = "");

/*! 
  Function checks if the file (group) is in data mode.
  If not, it places it in the data mode.
  While this is automatically done by the underlying C API
  for netCDF-4 files, the netCDF-3 files still need this call.
*/
void ncxxCheckDataMode(int ncid, std::string context = "");

#endif
