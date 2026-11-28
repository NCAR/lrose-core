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
/////////////////////////////////////////////////////////////
// Field.hh
//
// Class for representing characteristics of a field.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef Field_HH
#define Field_HH

#include <string>
#include <Mdv/Mdvx.hh>
#include "Params.hh"
#include "ChannelSet.hh"

using namespace std;

class Field {

public:
  
  typedef enum {
    COUNTS,
    RADIANCE,
    DEG_K,
    DEG_C,
    ALBEDO
  } output_units_t;
  
  // constructor
  
  Field(const Params::field_t &field,
        const ChannelSet &channel_set);
  
  // Destructor
  
  virtual ~Field();

  // get methods
  
  int getChannelId() const { return _channelId; }
  const Channel *getChannel() const { return _channel; }
  const string &getName() const { return _name; }
  output_units_t getOutputUnits() const { return _outputUnits; }
  Mdvx::encoding_type_t getOutputEncoding() const { return _outputEncoding; }
  
private:
  
  string _name;
  int _channelId;
  const Channel *_channel;
  output_units_t _outputUnits;
  Mdvx::encoding_type_t _outputEncoding;

};

#endif



