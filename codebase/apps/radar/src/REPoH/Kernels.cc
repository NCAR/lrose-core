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
void Kernels::print(void) const
{
  vector<KernelPair>::const_iterator i;
  for (i=_k.begin(); i!=_k.end(); ++i)
    i->print();
}

/*----------------------------------------------------------------*/
void Kernels::add(const KernelPair &k, const time_t &time, const double vlevel, 
		  const KernelGrids &grids, const Params &params,
		  const double dx, Grid2d &kcp)
{
  KernelPair k0(k);
  k0.finish_processing(time, vlevel, grids, params, dx, _next_available_id,
		       kcp);
  _k.push_back(k0);
  _next_available_id += 2; //  because the KernelPair uses up two i.d.s
}

/*----------------------------------------------------------------*/
string Kernels::humidity_estimate(const double vlevel, const GridProj &gp,
				  Grid2d &att, Grid2d &hum) const
{
  string ret;
  for (int i=0; i<(int)_k.size(); i++)
    ret += _k[i].humidity_estimate(vlevel, gp, att, hum);
  return ret;
}

/*----------------------------------------------------------------*/
bool Kernels::write_genpoly(const string &url, const time_t &time,
			    const int nx, const int ny,
			    const bool outside) const
{
  DsSpdb D;
  D.clearPutChunks();
  D.setPutMode(Spdb::putModeAdd);
  D.addUrl(url);
  bool stat = true;
  for (int i=0; i<(int)_k.size(); ++i)
    if (!_k[i].write_genpoly(time, nx, ny, outside, D))
      stat = false;
  return stat;
}
