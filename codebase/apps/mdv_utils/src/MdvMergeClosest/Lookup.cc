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
#include <toolsa/copyright.h>

/**
 * @file Lookup.cc
 */

#include "Lookup.hh"
#include <rapmath/usort.h>
#include <toolsa/LogMsg.hh>

//----------------------------------------------------------------
Lookup::Lookup(void) : _available(NULL), _values(NULL), _lookup(NULL),
		       _nx(0), _ny(0), _ninput(0)
{
}

//----------------------------------------------------------------
Lookup::~Lookup()
{
  if (_available != NULL)
  {
    delete [] _available;
  }
  if (_values != NULL)
  {
    delete [] _values;
  }
  if (_lookup != NULL)
  {
    delete [] _lookup;
  }
}

//----------------------------------------------------------------
void Lookup::init(const Parms &parms, const MdvxProj &proj)
{
  Mdvx::coord_t c = proj.getCoord();
  _nx = c.nx;
  _ny = c.ny;
  _ninput = parms._input.size();

  _values = new double[_nx*_ny*_ninput];
  _lookup = new int[_nx*_ny*_ninput];
  _available = new bool[_ninput];

  for (int i=0; i<_ninput; ++i)
  {
    _available[i] = false;

    // set up each input using its lat/lon
    int x0, y0;
    if (proj.latlon2xyIndex(parms._input[i]._latitude, 
			    parms._input[i]._longitude, 
			    x0, y0) != 0)
    {
      LOGF(LogMsg::ERROR, "Lat lon outside grid not implemented %s",
	   parms._input[i]._url.c_str());
      exit(1);
    }

    for (int y=0; y<_ny; ++y)
    {
      double dy = static_cast<double>(y-y0);
      for (int x=0; x<_nx; ++x)
      {
	double dx = static_cast<double>(x-x0);
	double d = dy*dy + dx*dx;
	_values[i*_nx*_ny + y*_nx + x] = d;
	_lookup[i*_nx*_ny + y*_nx + x] = i;
      }
    }
  }

  // now go back through and order the lookup values
  double *array = new double[_ninput];
  int *index = new int[_ninput];
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      // pull out values for each input
      for (int i=0; i<_ninput; ++i)
      {
	array[i] = _values[i*_nx*_ny + y*_nx + x];
	index[i] = i;
      }
      usort_index( array, _ninput, index );
      for (int i=0; i<_ninput; ++i)
      {
	_lookup[i*_nx*_ny + y*_nx + x] = index[i];
      }
    }
  }
  delete [] array;
  delete [] index;
}

//----------------------------------------------------------------
void Lookup::update(int index)
{
  _available[index] = true;
}

//----------------------------------------------------------------
void Lookup::timeout(int index)
{
  _available[index] = false;
}

//----------------------------------------------------------------
int Lookup::bestInput(int x, int y) const
{
  for (int i=0; i<_ninput; ++i)
  {
    int ind = _lookup[i*_nx*_ny + y*_nx + x];
    if (_available[ind])
    {
      return ind;
    }
  }
  return -1;
}

