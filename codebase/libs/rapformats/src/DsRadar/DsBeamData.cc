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
///////////////////////////////////////////////////////////////////////////
//  DsBeamData top-level application class
//
//////////////////////////////////////////////////////////////////////////
#include <cstring>
#include <rapformats/DsBeamData.hh>
using namespace std;

DsBeamData::DsBeamData(const int bytewidth, const int nfields,
		       const int ngates, const vector<DsBeamDataFieldParms> &p)
{
  _nfields = nfields;
  _ngates = ngates;
  _bytewidth = bytewidth;
  _parm = p;
  if (bytewidth == 1)
  {
    _f4ptr = NULL;
    _i2ptr = NULL;
    _i1ptr = new ui08[nfields*ngates];
  }
  else if (bytewidth == 2)
  {
    _f4ptr = NULL;
    _i2ptr = new ui16[nfields*ngates];
    _i1ptr = NULL;
  }
  else if (bytewidth == 4)
  {
    _f4ptr = new fl32[nfields*ngates];
    _i2ptr = NULL;
    _i1ptr = NULL;
  }
  else
  {
    _f4ptr = NULL;
    _i2ptr = NULL;
    _i1ptr = NULL;
  }
}

/*----------------------------------------------------------------*/
DsBeamData::DsBeamData(const DsBeamData &b)
{
  _parm = b._parm;
  _copy(b);
}

/*----------------------------------------------------------------*/
DsBeamData::~DsBeamData() 
{
  _free();
}

/*----------------------------------------------------------------*/
void DsBeamData::operator=(const DsBeamData &b)
{
  _parm = b._parm;
  if (_bytewidth != b._bytewidth || _nfields != b._nfields ||
      _ngates != b._ngates)
  {
    _free();
    _copy(b);
  }
  else
    _copy_contents(b);
}

/*----------------------------------------------------------------*/
bool DsBeamData::operator==(const DsBeamData &b) const
{
  return (_nfields == b._nfields &&
	  _ngates == b._ngates &&
	  _bytewidth == b._bytewidth &&
	  _parm == b._parm &&
	  _f4ptr == b._f4ptr &&
	  _i2ptr == b._i2ptr &&
	  _i1ptr == b._i1ptr);
}

/*----------------------------------------------------------------*/
ui08 *DsBeamData::get_ptr(void) const
{
  if (_i1ptr != NULL)
    return _i1ptr;
  if (_i2ptr != NULL)
    return (ui08 *)_i2ptr;
  if (_f4ptr != NULL)
    return (ui08 *)_f4ptr;
  return NULL;
}

/*----------------------------------------------------------------*/
void DsBeamData::fill_with_missing(void)
{
  for( int ifield = 0; ifield < _nfields; ifield++ )
  {
    int missing = _parm[ifield]._missing;
    for( int igate = 0; igate < _ngates; igate++ )
	_write(ifield, igate, missing);
  }
}

/*----------------------------------------------------------------*/
void DsBeamData::set_to_missing(const int ifield, const int igate)
{
  int missing = _parm[ifield]._missing;
  _write(ifield, igate, missing);
}

/*----------------------------------------------------------------*/
bool DsBeamData::is_missing_at(const int ifield, const int igate) const
{
  fl32 v = get_value(ifield, igate);
  return (v == (fl32)_parm[ifield]._missing);
}

/*----------------------------------------------------------------*/
int DsBeamData::num_missing(void) const
{
  int count = 0;
  for (int ifield=0; ifield<_nfields; ++ifield)
    for (int igate=0; igate<_ngates; ++igate)
    {
      if (is_missing_at(ifield, igate))
	++count;
    }
  return count;
}

/*----------------------------------------------------------------*/
void DsBeamData::copy(const DsRadarBeam &r)
{
  int dlen = r.getDataNbytes();
  void *data = r.getData();

  if (dlen > _nfields*_ngates*_bytewidth)
  {
    dlen = _nfields*_ngates*_bytewidth;
    printf("WARNING overflow in DsBeamData::set_values, truncated\n");
  }

  if (_f4ptr != NULL)
    memcpy((void *)_f4ptr, data, dlen);
  if (_i2ptr != NULL)
    memcpy((void *)_i2ptr, data, dlen);
  if (_i1ptr != NULL)
    memcpy((void *)_i1ptr, data, dlen);
}

/*----------------------------------------------------------------*/
bool DsBeamData::copy_contents(const DsBeamData &b)
{
  if (b._bytewidth != _bytewidth)
  {
    printf("ERROR in copy contents unequal bytewidth\n");
    return false;
  }
  if (b._nfields != _nfields)
  {
    printf("ERROR in copy contents unequal nfields\n");
    return false;
  }
  if (b._ngates != _ngates)
  {
    printf("ERROR in copy contents unequal ngates\n");
    return false;
  }

  int dlen = _nfields*_ngates*_bytewidth;
  if (_bytewidth == 4)
    memcpy((void *)_f4ptr, (void *)b._f4ptr, dlen);
  else if (_bytewidth == 2)
    memcpy((void *)_i2ptr, (void *)b._i2ptr, dlen);
  else if (_bytewidth == 1)
    memcpy((void *)_i1ptr, (void *)b._i1ptr, dlen);
  else
    return false;

  _parm = b._parm;
  return true;
}

/*----------------------------------------------------------------*/
fl32 DsBeamData::get_value(const int ifield,  const int igate) const
{
  if (_f4ptr != NULL)
    return _f4ptr[igate*_nfields + ifield];
  else if (_i2ptr != NULL)
    return (fl32)_i2ptr[igate*_nfields + ifield];
  else if (_i1ptr != NULL)
    return (fl32)_i1ptr[igate*_nfields + ifield];
  else
  {
    printf("ERROR DsBeamData::get_value, no data\n");
    return 0.0;
  }
}

/*----------------------------------------------------------------*/
ui16 DsBeamData::get_ui16_value(const int ifield,  const int igate) const
{
  if (_i2ptr != NULL)
    return _i2ptr[igate*_nfields + ifield];
  else
    return (ui16)_parm[ifield]._missing;
}

/*----------------------------------------------------------------*/
fl32 DsBeamData::get_scaled_value(const int ifield,  const int igate) const
{
  float v = (float)get_value(ifield, igate);
  return (fl32)_parm[ifield].scaled_value(v);
}

/*----------------------------------------------------------------*/
void DsBeamData::put_scaled_value(const fl32 f, const int ifield,
				  const int igate)
{
  // note this cast is safe 
  int v = (int)_parm[ifield].unscaled_value(f);
  _write(ifield, igate, v);
}

/*----------------------------------------------------------------*/
void DsBeamData::print_debug_value(const int ifield,  const int igate) const
{
  fl32 v = get_scaled_value(ifield, igate);
  fl32 u = get_value(ifield, igate);
  printf("scaledv:%6.2f rawv:%d (s,b):(%f,%f)", v,
	 (int)u, _parm[ifield]._scale, _parm[ifield]._bias);
}

/*----------------------------------------------------------------*/
void DsBeamData::print_values(const char *msg) const
{
  int count = 0;
  printf("%s\n", msg);
  for (int i=0; i<_nfields*_ngates; ++i)
  {
    _print_value(i);
    if (++count == 10)
    {
      count = 0;
      printf("\n");
    }
  }
}
      
/*----------------------------------------------------------------*/
void DsBeamData::print_values_with_index(const char *msg) const
{
  int count=0;
  printf("%s\n", msg);
  printf("%4d:", 0);
  for (int i=0; i<_nfields; ++i)
  {
    for (int g=0; g<_ngates; ++g)
    {
      _print_value(g*_nfields + i);
      if (++count == 10)
      {
	count = 0;
	printf("\n");
	printf("%4d:", g+1);
      }
    }
  }
  printf("End of %s\n", msg);
}  

/*----------------------------------------------------------------*/
void DsBeamData::print_scaled_values_with_index(const char *msg) const
{
  int count=0;
  printf("%s\n", msg);
  printf("%4d:", 0);
  for (int i=0; i<_nfields; ++i)
  {
    for (int g=0; g<_ngates; ++g)
    {
      if (is_missing_at(i, g))
	printf("MISS  ");
      else
      {
	fl32 v = get_scaled_value(i, g);
	printf("%5.2f ", v);
      }
      if (++count == 10)
      {
	count = 0;
	printf("\n");
	printf("%4d:", g+1);
      }
    }
  }
  printf("End of %s\n", msg);
}  

/*----------------------------------------------------------------*/
void DsBeamData::_write(int ifield, int igate, int value)
{
  if (_f4ptr != NULL)
    _f4ptr[igate*_nfields + ifield] = (fl32)value;
  else if (_i2ptr != NULL)
    _i2ptr[igate*_nfields + ifield] = (ui16)value;
  else if (_i1ptr != NULL)
    _i1ptr[igate*_nfields + ifield] = (ui08)value;
}


/*----------------------------------------------------------------*/
void DsBeamData::_free(void)
{
  if (_f4ptr != NULL)
  {
    delete [] _f4ptr;
    _f4ptr = NULL;
  }
  if (_i2ptr != NULL)
  {
    delete [] _i2ptr;
    _i2ptr = NULL;
  }
  if (_i1ptr != NULL)
  {
    delete [] _i1ptr;
    _i1ptr = NULL;
  }
  _bytewidth = 0;
}

/*----------------------------------------------------------------*/
void DsBeamData::_copy(const DsBeamData &b)
{
  _nfields = b._nfields;
  _ngates = b._ngates;
  _bytewidth = b._bytewidth;
  if (b._f4ptr != NULL)
  {
    _f4ptr = new fl32[_nfields*_ngates];
    _i2ptr = NULL;
    _i1ptr = NULL;
    memcpy(_f4ptr, b._f4ptr, sizeof(fl32)*_ngates*_nfields);
  }
  else if (b._i2ptr != NULL)
  {
    _i2ptr = new ui16[_nfields*_ngates];
    _f4ptr = NULL;
    _i1ptr = NULL;
    memcpy(_i2ptr, b._i2ptr, sizeof(ui16)*_ngates*_nfields);
  }
  else if (b._i1ptr != NULL)
  {
    _i1ptr = new ui08[_nfields*_ngates];
    _f4ptr = NULL;
    _i2ptr = NULL;
    memcpy(_i1ptr, b._i1ptr, sizeof(ui08)*_ngates*_nfields);
  }
  else
  {
    _f4ptr = NULL;
    _i2ptr = NULL;
    _i1ptr = NULL;
  }
}

/*----------------------------------------------------------------*/
void DsBeamData::_copy_contents(const DsBeamData &b)
{
  if (b._f4ptr != NULL)
    memcpy(_f4ptr, b._f4ptr, sizeof(fl32)*_ngates*_nfields);
  else if (b._i2ptr != NULL)
    memcpy(_i2ptr, b._i2ptr, sizeof(ui16)*_ngates*_nfields);
  else if (b._i1ptr != NULL)
    memcpy(_i1ptr, b._i1ptr, sizeof(ui08)*_ngates*_nfields);
}

/*----------------------------------------------------------------*/
void DsBeamData::_print_value(const int i) const
{
  if (_f4ptr != NULL)
    printf("%5.2f ", _f4ptr[i]);
  else if (_i2ptr != NULL)
    printf("%6d ", _i2ptr[i]);
  else if (_i1ptr != NULL)
    printf("%6d ", _i1ptr[i]);
  else
    printf("?? ");
}

