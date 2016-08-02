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

/**
 * @file Parms.cc
 */
#include "Parms.hh"

//------------------------------------------------------------------
Parms::Parms(const Params &p, const std::string &progName) : Params(p), 
							     _progName(progName)
{
}

//------------------------------------------------------------------
Parms::~Parms(void)
{
}

//------------------------------------------------------------------
bool Parms::hasSnr(void) const
{
  return strlen(snr_field) > 0;
}

//------------------------------------------------------------------
bool Parms::isMask(const std::string &name) const
{
  for (int i=0; i<output_fields_n; ++i)
  {
    if (_output_fields[i].name == name)
    {
      return _output_fields[i].type == Params::MASK;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool Parms::matchingOutput(Params::output_data_t type, std::string &name) const
{
  for (int i=0; i<output_fields_n; ++i)
  {
    if (_output_fields[i].type == type)
    {
      name = _output_fields[i].name;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
const Params::output_field_t *
Parms::matchingOutput(const std::string &name) const
{
  for (int i=0; i<output_fields_n; ++i)
  {
    if (_output_fields[i].name == name)
    {
      return &_output_fields[i];
    }
  }
  return NULL;
}
  

//------------------------------------------------------------------
const Params::rainrate_field_t *
Parms::matchingRainrate(const std::string &name) const
{
  for (int i=0; i<rainrate_fields_n; ++i)
  {
    if (_rainrate_fields[i].output_rainrate_name == name)
    {
      return &_rainrate_fields[i];
    }
  }
  return NULL;
}
  

//------------------------------------------------------------------
int Parms::numRainRate(void) const
{
  return rainrate_fields_n;
}

//------------------------------------------------------------------
std::string Parms::ithOutputRateName(int i) const
{
  return _rainrate_fields[i].output_rainrate_name;
}

//------------------------------------------------------------------
std::string Parms::ithInputPrecipName(int i) const
{
  return _rainrate_fields[i].input_precip_name;
}
  

