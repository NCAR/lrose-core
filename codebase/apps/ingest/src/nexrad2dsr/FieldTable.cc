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
//  $Id: FieldTable.cc,v 1.6 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>

#include "Driver.hh"
#include "FieldTable.hh"
using namespace std;


const size_t FieldTable::NUM_FIELD_TYPES = 4;

//
// The NEXRAD velocity scale and bias may vary for 0.5m/s and 1.0m/s data
// Here we arbitrarily initialize the velocity for 0.5m/s
// The scale and bias will be modified as necessary during data reformatting
//
const FieldTable::fieldTypeInfo_t
             FieldTable::fieldTypeInfo[NUM_FIELD_TYPES] = {   
             //
             // Field      Field
             //   Id        Name    Units   Scale   Bias
             //
             { VEL_DATA,   "VEL",   "m/s",   0.5,   -63.5 },
             { SPW_DATA,   "SPW",   "m/s",   0.5,   -63.5 },   
             { DBZ_DATA,   "DBZ",   "dBz",   0.5,   -32.0 },   
             { SNR_DATA,   "SNR",   "dB",    0.5,   -32.0 }};


FieldTable::FieldTable()
{
   size_t     i;
   FieldType *fieldType;

   //
   // Set the initial values for each of the data fields
   //
   for( i=0; i < NUM_FIELD_TYPES; i++ ) {
      fieldType = new FieldType( fieldTypeInfo[i].name,
                                 fieldTypeInfo[i].units,
                                 fieldTypeInfo[i].scale,
                                 fieldTypeInfo[i].bias);
      fieldTypes[fieldTypeInfo[i].id] = fieldType;
   }
}

FieldTable::~FieldTable()
{
   fieldTypeById::iterator i;

   //
   // Clear out the field info vector
   //
   for( i=fieldTypes.begin(); i != fieldTypes.end(); i++ ) {
      delete( (*i).second );
   }
   fieldTypes.clear();
}

int
FieldTable::init( Params& params )
{
   FieldType *fieldType = NULL;

   //
   // Make note of which fields are requested
   //
   POSTMSG( DEBUG, "Initializing the field table" );

   for( int i=0; i < params.output_fields_n; i++ ) {

      switch( params._output_fields[i] ) {

         case Params::DBZ:
              fieldType = getField( DBZ_DATA );
              break;
         case Params::VEL:
              fieldType = getField( VEL_DATA );
              break;
         case Params::SPW:
              fieldType = getField( SPW_DATA );
              break;
         case Params::SNR:
              fieldType = getField( SNR_DATA );
              break;
      }

      //
      // Set the field's request status to true
      //
      fieldType->setRequested();
   }

   return( 0 );
}

FieldType*
FieldTable::getField( fieldId_t type )
{
   fieldTypeById::iterator i = fieldTypes.find( type );

   if ( i != fieldTypes.end() ) {
      return( (*i).second );
   }
   else {
     //
     // Something was set up wrong in the field table initialization
     //
     POSTMSG( ERROR, "Something was set up wrong in the field table initialization." );
     exit(-1);
   }
}

FieldType*
FieldTable::operator[]( size_t index )
{
   size_t                   count;
   fieldTypeById::iterator  i;

   //
   // Simulate indexed sequential access from a map
   //
   for( count=0, i=fieldTypes.begin(); i != fieldTypes.end(); count++, i++ ) {
      if ( count == index ) {
         return( (*i).second );
      }
   }

   //
   // We reached the end of the dictionary without getting to the
   // specified index -- not supposed to happen
   //
   POSTMSG( ERROR, "Reached the end of the dictionary without getting to the specified index.");
   exit(-1);
}

void
FieldTable::setScaleBias( fieldId_t type, float factor )
{ 
   size_t i;
   float  scale = 1.0;
   float  bias  = 0.0;

   //
   // Get the default scale and bias
   //
   for( i=0; i < NUM_FIELD_TYPES; i++ ) {
      if ( fieldTypeInfo[i].id == type ) {
         scale = fieldTypeInfo[i].scale;
         bias  = fieldTypeInfo[i].bias;
         break;
      }
   }

   //
   // Change the scale and bias by the requested factor
   //
   getField( type )->setScaleBias( scale*factor, bias*factor );
}

void
FieldTable::clearAvailable()
{
   fieldTypeById::iterator i;

   for( i=fieldTypes.begin(); i != fieldTypes.end(); i++ ) {
      (*i).second->clearAvailable();
   }
}

