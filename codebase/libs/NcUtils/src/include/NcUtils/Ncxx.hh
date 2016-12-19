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
//  Additional methods have been added to return error conditions. 
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

//
// generic include file for the Ncxx C++ API
//

#include <string>
using namespace std;

extern "C" {
#include <netcdf.h>
}

#include <NcUtils/NcxxAtt.hh>
#include <NcUtils/NcxxByte.hh>
#include <NcUtils/NcxxChar.hh>
#include <NcUtils/NcxxCheck.hh>
#include <NcUtils/NcxxCompoundType.hh>
#include <NcUtils/NcxxDim.hh>
#include <NcUtils/NcxxDouble.hh>
#include <NcUtils/NcxxEnumType.hh>
#include <NcUtils/NcxxException.hh>
#include <NcUtils/NcxxFile.hh>
#include <NcUtils/NcxxFloat.hh>
#include <NcUtils/NcxxGroup.hh>
#include <NcUtils/NcxxGroupAtt.hh>
#include <NcUtils/NcxxInt.hh>
#include <NcUtils/NcxxInt64.hh>
#include <NcUtils/NcxxOpaqueType.hh>
#include <NcUtils/NcxxShort.hh>
#include <NcUtils/NcxxString.hh>
#include <NcUtils/NcxxType.hh>
#include <NcUtils/NcxxUbyte.hh>
#include <NcUtils/NcxxUint.hh>
#include <NcUtils/NcxxUint64.hh>
#include <NcUtils/NcxxUshort.hh>
#include <NcUtils/NcxxVar.hh>
#include <NcUtils/NcxxVarAtt.hh>
#include <NcUtils/NcxxVlenType.hh>

class Ncxx {

public:
  static const double missingDouble;
  static const float missingFloat;
  static const int missingInt;
  static const unsigned char missingUchar;
  
  ////////////////////////////////////////
  // convert type enum to string
  
  static string ncTypeToStr(nc_type nctype);
  
};

  
