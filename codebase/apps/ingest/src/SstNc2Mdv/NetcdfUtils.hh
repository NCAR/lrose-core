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
/*
-------------------------------------------------------------------------------
NetcdfUtils.hh - Header for NetcdfUtils.cc

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
December 2004
-------------------------------------------------------------------------------
*/

#ifndef NetcdfUtils_H
#define NetcdfUtils_H

//
// SYSTEM INCLUDES
//

#include <netcdf.hh>
#include <netcdfcpp.h>

//
// LIBRARY INCLUDES
//

#include <toolsa/MsgLog.hh>

//
// LOCAL INCLUDES
//

//#include "NtdaGlobals.hh"

typedef
struct
   {
   string  callerID;     // name of calling routine
   MsgLog  *log;         // pointer to message log object
   FILE    *diag;        // pointer to diagnostic output file
   int     diag_lvl;     // diagnostic output level
   }
NcuPrtArgs_t;

// "NcuTypeNums_t" is an enumeration that determines the order
// of the NcuTypeNames list of type names
//typedef
//enum
//   {
//   NCU_Byte   = ncByte,
//   NCU_Char   = ncChar,
//   NCU_Short  = ncShort,
//   NCU_Int    = ncInt,
//   NCU_Float  = ncFloat,
//   NCU_Double = ncDouble
//   }
//NcuTypeNums_t;
//static const char
//*avoid_not_used_err = NcuTypeNames[0];

//   NcFile  *ncfile;      // pointer to netcdf input file
//   string  nc_var_name;  // name of netcdf variable to be read

namespace NetcdfUtils
{ // begin namespace NetcdfUtils

//
// NcTypeName
//    Return string containing name of ncType
//

template< typename ArgType >
   static string
   NcTypeName( ArgType aNcType );

//
// ElementSize
//

template< typename ArgType >
   int
   ElementSize( ArgType aNcType );

//
// LoadNcVar
//    Allocate space for and read in a netcdf variable
//    Calls "NetcdfUtils::ReadNcVar" to actually read the data
//

template< typename NcVarElemType >
int
LoadNcVar
   (
   const NcuPrtArgs_t  &arPrtArgs,    // const ref to print args
   const NcFile        &arNcFile,     // const ref to netcdf file
   const string        &arVarName,    // const ref to variable name
   NcVarElemType       **arpNcVarData // pointer to pointer to data
   );

//
// ReadNcVar
//    Read in a netcdf variable; requires pointer to block of memory
//    large enough to hold all of variable's data.
//

template< typename NcVarElemType >
int
ReadNcVar
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   const NcFile        &arNcFile,   // const ref to netcdf file
   const string        &arVarName,  // const ref to variable name
   NcVarElemType       *apNcVarData // pointer to variable receiving data
   );

//
// AddVarAtt
//
// NOTE: this function is only intended for use with "VoidPtr" = void *
// Had to make it a template function in order to work correctly
// within namespace "NetcdfUtils"


template< typename VoidPtr >
int
AddVarAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   NcVar               *apNcVar,    // const pointer to netcdf variable
   NcToken             aAttName,    // attribute name
   NcType              aAttType,    // attribute type
   int                 aNumVals,    // number of attribute values
   VoidPtr             *aAttVals    // pointer to block of values
   );

//
// AddGlobalAtt
//
// NOTE: this function is only intended for use with "VoidPtr" = void *
// Had to make it a template function in order to work correctly
// within namespace "NetcdfUtils"
//

template< typename VoidPtr >
int
AddGlobalAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   NcFile              *apNcFile,   // const pointer to netcdf variable
   const string        &arAttName,  // const ref to attribute name
   NcType              aAttType,    // attribute type
   long int            aNumVals,    // number of attribute values
   VoidPtr             *aAttVals    // pointer to block of values
   );

//
// CopyVarAtt
//    Copy a variable attribute (as opposed to a global attribute)
//    from one NcVar object to another NcVar object WITHOUT LEAKING MEMORY!
//
// NOTE: this function is only intended for use with CharPtr = char *
// Had to make it a template function in order to work correctly
// within namespace "NetcdfUtils"
//

template< typename CharPtr >
void
CopyVarAtt
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   CharPtr             *aAttName,
   NcVar               *apInputNcVar,
   NcVar               *apOutputNcVar
   );

//
// GetScalarAttValue
//

template< typename AttType >
void
GetScalarAttValue
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   const char          *aAttName,
   NcVar               *apInputNcVar,
   AttType             *apAttValue
   );

//
// WriteVarData
//
// NOTE: this function is only intended for use with VoidPtr = void *
// Had to make it a template function in order to work correctly
// within namespace "NetcdfUtils"

template< typename VoidPtr >
int
WriteVarData
   (
   const NcuPrtArgs_t  &arPrtArgs,  // const ref to print args
   NcFile              &arNcFile,   // ref to netcdf file
   const char          *aVarName,   // const ptr to variable name
   long int            *aEdges,     // edges array
   VoidPtr             *aDataPtr    // ptr to data block
   );

} // end namespace NetcdfUtils

//
// Abbreviate "NetcdfUtils" as "NCU"
//

namespace NCU = NetcdfUtils;

#include "NetcdfUtils.cc"

#endif
