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
//  $Id: FieldType.hh,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _FIELD_TYPE_INC_
#define _FIELD_TYPE_INC_

#include <rapformats/DsFieldParams.hh>

#include "Params.hh"
#include "Status.hh"
using namespace std;


class FieldType
{
public:

  FieldType(const char* fieldName, const char* fieldUnits,
            float fieldScale, float fieldBias );
  ~FieldType();

   //
   // Setting and fetching field state
   //
   void            setRequested( bool status=true ){ requested = status; }
   bool            isRequested( int* position = NULL );

   void            setInStream( bool status=true ){ inStream = status; }
   bool            isInStream(){ return inStream; }

   void            setStored( bool status=true ){ stored = status; }
   bool            isStored(){ return stored; }

   void            setPosition( int pos ){ position = pos; }
   int             getPosition(){ return position; }

   //
   // For convenience, we consider a field to be available
   // if it is either in the data stream or stored
   //
   void            clearAvailable(){ inStream = false; stored = false; }
   bool            isAvailable(){ return( inStream || stored ); }

   //
   // A field is writable only if it is both requested and available
   //
   bool            isWritable(){ return( requested && isAvailable() ); }

   //
   // Changing the scale and bias
   //
   void            setScaleBias( float scale, float bias )
                               { fieldParams->scale = scale;
                                 fieldParams->bias  = bias; }

   //
   // Return the DsFieldParams for this field of data
   //
   DsFieldParams*  getFieldParams(){ return( fieldParams ); }

private:

   bool            requested;
   bool            inStream;
   bool            stored;
   int             position;

   DsFieldParams  *fieldParams;
};

#endif
