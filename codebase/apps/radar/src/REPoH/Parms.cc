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

/*----------------------------------------------------------------*/
#include "Parms.hh"
#include <FiltAlg/ParmFiltAlgApp.hh>

/*----------------------------------------------------------------*/
Parms::Parms() : FiltAlgParms()
{
}

/*----------------------------------------------------------------*/
Parms::Parms(int argc, char **argv) : FiltAlgParms()
{
  if (!parmFiltAlgAppSet(_main, *this, argc, argv))
    exit(-1);
}

/*----------------------------------------------------------------*/
Parms::~Parms()
{
}

/*----------------------------------------------------------------*/
bool Parms::name_is_humidity(const string &name)
{
  if (name == "HUMIDITY")
    return true;
  else
  {
    printf("Bad name for humidity filter %s, name should = HUMIDITY\n",
	   name.c_str());
    return false;
  }
}

/*----------------------------------------------------------------*/
int
Parms::app_max_elem_for_filter(const FiltAlgParams::data_filter_t f) const
{
  string s = f.app_filter_name;
  if (name_is_humidity(s))
    return 1;
  else 
 {
    printf("ERROR unkown app filter parm name %s\n", s.c_str());
    return 0;
 }
}

/*----------------------------------------------------------------*/
const void *Parms::app_parms(void) const
{
  return (void *)&_main;
}
