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
//  Class for managing a keyed table (dictionary) of field types
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: FieldTable.hh,v 1.5 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _FIELD_TABLE_INC_
#define _FIELD_TABLE_INC_

#include <map>

#include "Params.hh"
#include "Status.hh"
#include "FieldType.hh"
using namespace std;

// 
// Dictionary of field types keyed by field type id
//
typedef   map< int, FieldType*, less<int> > fieldTypeById;


class FieldTable
{
public:
   FieldTable();
  ~FieldTable();

   //
   // All types of field data
   //
   enum fieldId_t { VEL_DATA,
                    SPW_DATA,
                    DBZ_DATA,
                    SNR_DATA };

   //
   // Return 0 upon success, -1 upon failure
   //
   int          init( Params& params );

   //
   // Set information on fields
   //
   void         setRequested( fieldId_t type )
                   { getField( type )->setRequested(); }

   void         setInStream( fieldId_t type )
                   { getField( type )->setInStream(); }

   void         setStored( fieldId_t type )
                   { getField( type )->setStored(); }

   //
   // Return flags regarding status of fields
   //
   bool         isRequested( fieldId_t type, int *position = NULL )
                  { return( getField( type )->isRequested( position )); }

   bool         isInStream( fieldId_t type )
                  { return( getField( type )->isInStream()); }

   bool         isStored( fieldId_t type )
                  { return( getField( type )->isStored()); }

   //
   // For convenience, we consider a field to be available
   // if it is either in the data stream or stored
   //
   bool         isAvailable( fieldId_t type )
                           { return( getField( type )->isAvailable()); }

   void         clearAvailable();

   //
   // Setting the scale & bias by applying a multiplicative factor
   // to the default scale and bias
   //
   void         setScaleBias( fieldId_t type, float factor );

   //
   // Because of the way the nexrad data comes with reflectivity and
   // velocity arriving in separate tilts for the lowest two elevation angles,
   // we have three different input scenarios that can occur:
   // 
   // 1. reflectivity data only         (1000m gate spacing)
   // 2. velocity & spectrumWidth data  (250m gate spacing)
   // 3. refl, vel, and spw             (250m gate spacing)
   //
   // NOTE: since spectrum width data is a derivative of velocity data,
   //       and signal/noise is a derivative of reflectivity data,
   //       we use the simplified nomenclature for these three cases:
   // 
   // 1. REFLECTIVITY ONLY
   // 2. VELOCITY ONLY
   // 3. all fields are in the stream
   //
   bool   velocityOnly(){ return( !isInStream( DBZ_DATA )); }
   bool   reflectivityOnly(){ return( !isInStream( VEL_DATA )); }

   //
   // Indexed sequential access to the dictionary of field information
   //
   size_t         size(){ return( fieldTypes.size() ); }
   FieldType*     operator[]( size_t index );

   //
   // Keyed random access to the dictionary of field information
   //
   FieldType*     getField( fieldId_t type );

   //
   // Initialization struct for supported field types
   //
   typedef struct { fieldId_t  id;
                    char*      name;
                    char*      units;
                    float      scale;
                    float      bias;
                  } fieldTypeInfo_t;

   static const fieldTypeInfo_t fieldTypeInfo[];

private:

   //
   // Dictionary of field types
   //
   fieldTypeById  fieldTypes;

   static const size_t          NUM_FIELD_TYPES;

};

#endif
