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

Module:	TempData

Author:	Dave Albo

Date:	Thu Jan 26 16:43:33 2006

Description:   Holds temporary data..needed for combine filter

************************************************************************/

/* System include files / Local include files */
#include <cstdio>
#include "TempData.hh"
using namespace std;

/* Constant definitions / Macro definitions / Type definitions */

/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
TempData::TempData(void)
{
  _data = NULL;
  _ndata = _nx = _ny = _nz = 0;
  _missing = _bad = 0.0;
}

/*----------------------------------------------------------------*/
TempData::TempData(const TempData &b)
{
  _data = b._data;
  _ndata = b._ndata;
  _nx = b._nx;
  _ny = b._ny;
  _nz = b._nz;
  _missing = b._missing;
  _bad = b._bad;
}

/*----------------------------------------------------------------*/
TempData::~TempData()
{
  if (_ndata > 0)
    free(_data);
}

/*----------------------------------------------------------------*/
void TempData::operator=(const TempData &b)
{
  if (_ndata > 0)
    free(_data);
  _data = b._data;
  _ndata = b._ndata;
  _nx = b._nx;
  _ny = b._ny;
  _nz = b._nz;
  _missing = b._missing;
  _bad = b._bad;
}

/*----------------------------------------------------------------*/
bool TempData::operator==(const TempData &b) const
{
  return (_data == b._data &&
	  _ndata == b._ndata &&
	  _nx == b._nx &&
	  _ny == b._ny &&
	  _nz == b._nz &&
	  _missing == b._missing &&
	  _bad == b._bad);
}

/*----------------------------------------------------------------*/
void TempData::print(void) const
{
  printf("TEMPDATA ndata=%d (%d,%d,%d) missing=%f  bad=%f  data=%ld\n", 
	 _ndata, _nx, _ny, _nz, _missing, _bad, (unsigned long int)_data);

  int i, j;
  if (_ndata > 0)
  {    
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
}    

/*----------------------------------------------------------------*/
void TempData::clear(void)
{
  if (_ndata > 0)
    free(_data);
  _data = NULL;
  _ndata = _nx = _ny = _nz = 0;
  _bad = _missing = 0;
}

/*----------------------------------------------------------------*/
void TempData::store(Data &D)
{
  if (_ndata > 0)
    free(_data);
  _data = (fl32 *)calloc(D._ndata, sizeof(fl32));
  _ndata = D._ndata;
  _nx = D._nx;
  _ny = D._ny;
  _nz = D._nz;
  _missing = D._missing;
  _bad = D._bad;
  memcpy(_data, D._data, _ndata*sizeof(fl32));
}

/*----------------------------------------------------------------*/
bool TempData::extract(Data &D)
{
  if (_ndata != D._ndata)
    return false;
  if (_ndata == 0)
    return false;

  memcpy(D._data, _data, _ndata*sizeof(fl32));
  D._ndata = _ndata;
  D._missing = _missing;
  D._bad = _bad;
  D._nx = _nx;
  D._ny = _ny;
  D._nz = _nz;
  return true;
}

/*----------------------------------------------------------------*/
fl32 *TempData::getVals(fl32 &missing, fl32 &bad, int &num) const
{
  missing = _missing;
  bad = _bad;
  num = _ndata;
  return _data;
}

/*----------------------------------------------------------------*/
void TempData::putVals(fl32 missing, fl32 bad)
{
  fl32 *d;
  int k;
  for(k=0,d=_data; k < _ndata; k++,d++)
  {
    if (*d == _missing)
      *d = missing;
    if (*d == _bad)
      *d = bad;
  }
  _missing = missing;
  _bad = bad;
}
