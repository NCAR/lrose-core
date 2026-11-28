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
//////////////////////////////////////////////////////////
// Field.cc
//
// Class for representing characteristics of a field.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include "Field.hh"

// Constructor

Field::Field(const Params::field_t &field,
             const ChannelSet &channel_set) :
        _channel(NULL)
  
{

  _name = field.name;
  _channelId = field.channel_id;
  _channel = channel_set.getChannel(_channelId);

  switch (field.output_units) {
    case Params::RADIANCE:
      _outputUnits = RADIANCE;
      break;
    case Params::DEG_K:
      _outputUnits = DEG_K;
      break;
    case Params::DEG_C:
      _outputUnits = DEG_C;
      break;
    case Params::ALBEDO:
      _outputUnits = ALBEDO;
      break;
    case COUNTS:
    default:
      _outputUnits = COUNTS;
      break;
  }

  switch (field.output_encoding) {
    case Params::OUT_INT8:
      _outputEncoding = Mdvx::ENCODING_INT8;
      break;
    case Params::OUT_INT16:
      _outputEncoding = Mdvx::ENCODING_INT16;
      break;
    case Params::OUT_FLOAT32:
      _outputEncoding = Mdvx::ENCODING_FLOAT32;
      break;
    default:
      _outputEncoding = Mdvx::ENCODING_ASIS;
  }

}

// Destructor

Field::~Field()

{

}

