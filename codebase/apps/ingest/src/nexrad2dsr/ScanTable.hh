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
////////////////////////////////////////////////////////////////////////////////
//
//  Class for managing a keyed table (dictionary) of scan types
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: ScanTable.hh,v 1.9 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _SCAN_TABLE_INC_
#define _SCAN_TABLE_INC_

#include <map>

#include "Params.hh"
#include "Status.hh"
#include "ScanType.hh"
using namespace std;

//
// Dictionary of scan types keyed by scan type id
//
typedef   map< int, ScanType*, less<int> > scanTypeById;


class ScanTable
{
public:
   ScanTable();
  ~ScanTable();

   //
   // Return 0 upon success, -1 upon failure
   //
   int      init( Params& params );

   //
   // Access methods to scan type information
   //
   ScanType*      getScanType( int typeId );

   const string   getName( int typeId )
                         { return( getScanType( typeId )->getName()); }

   float          getPulseWidth( int typeId )
                         { return( getScanType( typeId )->getPulseWidth()); }

   //
   // Nexrad elevation number (elevNum) is 1-based, not 0-based
   //
   float    getElev( int typeId, size_t elevNum )
                   { return( getScanType( typeId )->getElev( elevNum )); }


private:

   //
   // Dictionary of scan types
   //
   scanTypeById  scanTypes;

   //
   // Initialization constants for supported scan types, a.k.a. VCP
   //
   static const size_t  NUM_SCAN_TYPES = 10;

   static const float vcp32[];
   static const float vcp31[];
   static const float vcp21[];
   static const float vcp11[];
   static const float vcp12[];
   static const float vcp121[];
   static const float vcp82[];

   static const float vcp221[];
   static const float vcp211[];
   static const float vcp212[];

   typedef struct { int     id;
                    const   char* name;
                    float   pulseWidth;
                    size_t  numElev;
                    float*  elevList;
                  } scanTypeInfo_t;

   static const scanTypeInfo_t scanTypeInfo[];

};

#endif
