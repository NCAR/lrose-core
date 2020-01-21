// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>

/**
 * @file DataGrid.cc
 */

#include "DataGrid.hh"
#include "SimMath.hh"
#include <cmath>
#include <cstdio>
using std::vector;

//----------------------------------------------------------------
DataGrid::DataGrid(void) : _nx(0), _ny(0), _nz(0), _data(NULL), _missing(0)
{
}

//----------------------------------------------------------------
DataGrid::DataGrid(const int nx, const int ny, const int nz,
		   const fl32 missing, fl32 *data) :
  _nx(nx), _ny(ny), _nz(nz), _data(data), _missing(missing)
{
}

//----------------------------------------------------------------
DataGrid::~DataGrid()
{
}

//----------------------------------------------------------------
void DataGrid::setValue(int x, int y, int z, fl32 v, bool motion)
{
  if (motion)
  {
    _data[_nx*_ny*z + _nx*y + x] = v/KNOTS_TO_MS;
  }
  else
  {
    _data[_nx*_ny*z + _nx*y + x] = v;
  }
}

//----------------------------------------------------------------
void DataGrid::clear(void)
{
  for (int i=0; i< _nx*_ny*_nz; ++i)
  {
    _data[i] = _missing;
  }
}

