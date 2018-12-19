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
 * @file Kernels.cc
 */
#include "Kernels.hh"
#include <euclid/Grid2d.hh>
#include <Spdb/DsSpdb.hh>

/*----------------------------------------------------------------*/
Kernels::Kernels(void)
{
  _next_available_id = 1;
}

/*----------------------------------------------------------------*/
Kernels::~Kernels()
{
}

/*----------------------------------------------------------------*/
bool Kernels::getFloat(double &f) const
{
  return false;
}

/*----------------------------------------------------------------*/
void Kernels::print(void) const
{
  vector<KernelPair>::const_iterator i;
  for (i=_k.begin(); i!=_k.end(); ++i)
    i->print();
}

/*----------------------------------------------------------------*/
void Kernels::computeAttenuation(double dx, Grid2d &att) const
{
  att.setAllMissing();
  for (size_t i=0; i<_k.size(); ++i)
  {
    _k[i].computeAttenuation(dx, att);
  }
}
/*----------------------------------------------------------------*/
void Kernels::computeHumidity(double dx, Grid2d &hum) const
{
  hum.setAllMissing();
  for (size_t i=0; i<_k.size(); ++i)
  {
    _k[i].computeHumidity(dx, hum);
  }
}

/*----------------------------------------------------------------*/
string Kernels::asciiOutput(double vlevel, const MdvxProj &gp) const
{
  string ret;
  for (int i=0; i<(int)_k.size(); i++)
    ret += _k[i].asciiOutput(vlevel, gp);
  return ret;
}  

/*----------------------------------------------------------------*/
void Kernels::filterToGood(void)
{
  std::vector<KernelPair>::iterator i;
  for (i=_k.begin(); i!= _k.end(); )
  {
    if (i->passedTests())
    {
      i++;
    }
    else
    {
      i = _k.erase(i);
    }
  }
}

/*----------------------------------------------------------------*/
bool Kernels::writeGenpoly(const string &url, const time_t &time,
			   bool outside, const MdvxProj &proj) const
{
  DsSpdb D;
  D.clearPutChunks();
  D.setPutMode(Spdb::putModeAdd);
  D.addUrl(url);
  bool stat = true;
  for (int i=0; i<(int)_k.size(); ++i)
    if (!_k[i].writeGenpoly(time, outside, proj, D))
      stat = false;
  return stat;
}
