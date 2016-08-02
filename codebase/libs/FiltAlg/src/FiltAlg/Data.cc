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
 * @file Data.cc
 */
#include <FiltAlg/Data.hh>

//------------------------------------------------------------------
Data::Data() : VlevelData("unknown", true, false), Data1d()
{
  _name = "unknown";
  _type = UNKNOWN;
  _is_output = false;
}

//------------------------------------------------------------------
Data::Data(const string &name, const Data_t type, const bool is_output) :
  VlevelData(name, type == DATA1D, type == GRID3D), Data1d()
{
  _name = name;
  _type = type;
  _is_output = is_output;
  if (_type == DATA1D)
  {
    set_1d_name(name);
  }
}

//------------------------------------------------------------------
Data::~Data()
{
}

//------------------------------------------------------------------
void Data::print(void) const
{
  printf("Data %s %s output:%d\n", _name.c_str(), print_type(_type).c_str(),
	 _is_output);
  if (_type != DATA1D)
  {
    print_vlevel_data();
  }
  else
  {
    print_1d();
  }
}

//------------------------------------------------------------------
string Data::print_type(const Data_t type)
{
  string ret;
  switch (type)
  {
  case GRID3D:
    ret = "GRID3D";
    break;
  case DATA2D:
    ret = "DATA2D";
    break;
  case DATA1D:
    ret = "DATA1D";
    break;
  default:
    ret = "UNKNOWN";
    break;
  }
  return ret;
}

