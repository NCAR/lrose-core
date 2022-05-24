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
#include <cstdlib>

/************************************************************************

Module:	Data

Author:	Dave Albo

Date:	Fri Jan 27 16:56:22 2006

Description:   Holds limited data info locally

************************************************************************/



/* System include files / Local include files */
#include <cstdio>
#include "Data.hh"
#include "Algs.hh"
using namespace std;

/* Constant definitions / Macro definitions / Type definitions */

/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
Data::Data(MdvxField *f)
{
  // create the data from the input.
  Mdvx::field_header_t h = f->getFieldHeader();
  _data = (fl32 *)f->getVol();
  _nx = h.nx;
  _ny = h.ny;
  _nz = h.nz;
  _ndata = _nx*_ny*_nz;
  _missing = h.missing_data_value;
  _bad = h.bad_data_value;
}

/*----------------------------------------------------------------*/
Data::Data(const Data &b)
{
  _data = b._data;
  _nx = b._nx;
  _ny = b._ny;
  _nz = b._nz;
  _ndata = b._ndata;
  _missing = b._missing;
  _bad = b._bad;
}

/*----------------------------------------------------------------*/
Data::~Data()
{
}

/*----------------------------------------------------------------*/
void Data::operator=(const Data &b)
{
  _data = b._data;
  _nx = b._nx;
  _ny = b._ny;
  _nz = b._nz;
  _ndata = b._ndata;
  _missing = b._missing;
  _bad = b._bad;
}

/*----------------------------------------------------------------*/
bool Data::operator==(const Data &b) const
{
  return (_data == b._data &&
	  _nx == b._nx &&
	  _ny == b._ny &&
	  _nz == b._nz &&
	  _ndata == b._ndata &&
	  _missing == b._missing &&
	  _bad == b._bad);
}

/*----------------------------------------------------------------*/
void Data::print(void) const
{    
  int i,j;
  printf("DATA nx,y,z:%d,%d,%d, ndata=%d  missing=%f  bad=%f  data=%ld\n", 
	 _nx, _ny, _nz, _ndata, _missing, _bad, (unsigned long int)_data);
  for (i=0,j=0; i< _ndata; i+=_ndata/100)
  {
    printf("%6.4f ", _data[i]);
    if (++j>10)
    {
      j=0;
      printf("\n");
    }
  }
}

/*----------------------------------------------------------------*/
bool Data::evaluate(double &min, double &max, double &mean) const
{
  Algs A;
  return A.evaluate(_data, _ndata, _missing, _bad, min, max, mean);
}

/*----------------------------------------------------------------*/
void Data::change(fl32 newbad)
{
  for(int k=0; k < _ndata; k++)
  {
    if (_data[k] == _missing || _data[k] == _bad)
      _data[k] = newbad;
  }
  _missing = _bad = newbad;
}

/*----------------------------------------------------------------*/
void Data::filter_thresh(Params::mdv_thresh_parm_t &p)
{
  Algs A;
  A.filter_thresh(_data, _ndata, _missing, _bad, p);
}

/*----------------------------------------------------------------*/
void Data::filter_passthrough(void)
{
}

/*----------------------------------------------------------------*/
void Data::filter_smooth(Params::mdv_smooth_parm_t &p)
{
  Algs A;
  A.filter_smooth(_data, _nx, _ny, _nz, _ndata, _missing, _bad, p);
}

/*----------------------------------------------------------------*/
bool Data::synchronize_data(fl32 *data, int n, fl32 missing,
			    fl32 bad, fl32 &newbad)
{
  Algs A;
  if (A.needs_synch(_data, _ndata, _missing, _bad, data, n, missing,
		    bad, newbad))
  {
    change(newbad);
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
void Data::filter_combine(fl32 *data, fl32 missing, fl32 bad, int n,
			  Params::mdv_combine_parm_t &p)
{
  Algs A;
  A.filter_combine(_data, _ndata, _missing, _bad, data, n, missing, bad, p);
}

/*----------------------------------------------------------------*/
void Data::filter_expand_value(Params::mdv_expand_value_parm_t &p)
{
  // Copy local data into Buffer.
  fl32 *Buffer = (fl32 *)calloc(_ndata, sizeof(fl32));
  memcpy(Buffer, _data, _ndata*sizeof(fl32));

  // filter down to p.Value (more or less.)
  Algs A;
  A.filter_thresh(Buffer, _ndata, _missing, _bad, p.Value-p.Tol, p.Value+p.Tol,
		  false, 0.0);

  // now expand using smoothing method = SMOOTH_MAX.
  A.filter_smooth(Buffer, _nx, _ny, _nz, _ndata, _missing, _bad, p.HalfWin,
		  Params::SMOOTH_MAX);

  // now put this data back into original, overwriting what is there.
  // Note do NOT need to synch up data since missing and bad have not been
  // modified yet, so _data and Buffer are in synch.
  A.filter_overwrite(_data, _ndata, _missing, _bad, Buffer, _ndata, _missing,
		     _bad);
  free(Buffer);
}

