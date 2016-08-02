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
//  Abstract base class for EDR decoders
//
//  Sue Dettling RAP, NCAR, Boulder, CO, 80307, USA
//  November 2004
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _EDR_DECODER_INC_
#define _EDR_DECODER_INC_

#include "Params.hh"
#include "Status.hh"
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
using namespace std;

class EdrDecoder
{
public:
   EdrDecoder(){}
   virtual ~EdrDecoder(){};

   //
   // Optional initialization for the sub-classes
   // Return 0 upon success, -1 upon failure
   //
   virtual int init( Params& params ){ return 0; }

   //
   // Sub-classes are required to provide a decode method
   //
   virtual Status::info_t decodeEdrMsg( ui08* buffer, DateTime msgTime ) = 0;
   virtual Status::info_t decodeAsciiMsg( ui08* buffer, DateTime msgTime ) = 0;
   virtual Status::info_t decodeBufrMsg( ui08* buffer, DateTime msgTime ) = 0;
   
};

#endif



