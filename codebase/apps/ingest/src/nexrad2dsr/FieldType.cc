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
//  Class for containing field information
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: FieldType.cc,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include "FieldType.hh"
using namespace std;

FieldType::FieldType( char* fieldName, char* fieldUnits,
                      float fieldScale, float fieldBias )
{
   //
   // Create the DsFieldParams which is needed for
   // writing out the requested field of data
   //
   // NOTE: This class does not own the newly created fieldParam.
   //       DsRadarMsg deletes any field params which are pushed back 
   //       as part of the DsFieldParams vector of the DsRadarMsg.
   //       And the application class Reformat does some intermediate
   //       clearing out of these field params.
   //
   fieldParams = new DsFieldParams( fieldName, fieldUnits,
                                    fieldScale, fieldBias );

   //
   // The management of these state members is the main function of this class
   // The state is used by the Reformat class to determine which data 
   // fields are to be reformatted for any particular DsRadarMsg
   //
   requested = false;
   inStream  = false;
   stored    = false;
   position  = -1;
}

FieldType::~FieldType()
{
   delete fieldParams;
}

bool
FieldType::isRequested( int *posPtr )
{
   //
   // Option return of field position along with the requested flag
   //
   if ( posPtr != NULL ) {
      *posPtr = position;
   }

   return( requested );
}
