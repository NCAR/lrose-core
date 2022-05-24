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
 * @file Grid2d.c
 * @brief two dimensional data grid
 */

#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>

//---------------------------------------------------------------------------
Grid2d::Grid2d(void) : _name("Unknown"), _missing(0.0), _npt(0), _nx(0), _ny(0)
{
}

//---------------------------------------------------------------------------
Grid2d::Grid2d(const std::string &name)  : _name(name), _missing(0.0), _npt(0),
					   _nx(0), _ny(0)
{
}

//---------------------------------------------------------------------------
Grid2d::Grid2d(const std::string &name, double missing) : _name(name),
							 _missing(missing),
							 _npt(0), _nx(0), _ny(0)
{
}

//---------------------------------------------------------------------------
Grid2d::Grid2d(const std::string &name, int nx, int ny, double missing) :
  _name(name), _missing(missing), _npt(nx*ny), _nx(nx), _ny(ny)
{
  for (int i=0; i<nx*ny; ++i)
  {
    _data.push_back(missing);
  }
}

//---------------------------------------------------------------------------
Grid2d::Grid2d(const std::string &name, int nx, int ny, 
	       const std::vector<double> &data, double missing) :
  _name(name), _data(data), _missing(missing), _npt(nx*ny), _nx(nx), _ny(ny)
{
}

//---------------------------------------------------------------------------
Grid2d::~Grid2d(void)
{

}

//---------------------------------------------------------------------------
bool Grid2d::dimensionsEqual(const Grid2d &g) const
{
  return (_nx == g._nx && _ny == g._ny);
}

//---------------------------------------------------------------------------
bool Grid2d::valuesEqual(const Grid2d &g) const
{
  return (_nx == g._nx && _ny == g._ny && _npt == g._npt && 
	  _missing == g._missing && _data == g._data);
}

//---------------------------------------------------------------------------
void Grid2d::print(void) const
{
  printf("Grid2d:%s miss:%.2f npt:%d nx,ny:%d,%d\n",
	 _name.c_str(), _missing, _npt, _nx, _ny);
}

//---------------------------------------------------------------------------
void Grid2d::log(void) const
{
  LOG(PRINT) << "\tGrid2d:" << _name << " miss:" << _missing << " npt:"
	     << _npt << "nx,ny:" << _nx << "," << _ny;
}

//---------------------------------------------------------------------------
void Grid2d::print2(const std::string &n, int i) const
{
  printf("%s Grid2d[%d]:%s miss:%.2f npt:%d nx,ny:%d,%d\n",
	 n.c_str(), i, _name.c_str(), _missing, _npt, _nx, _ny);
}

//---------------------------------------------------------------------------
void Grid2d::printRange(int x0, int x1, int y0, int y1) const
{
  for (int y=y0; y<=y1; ++y)
  {
    printf("y=%d\n", y);
    int count = 0;
    for (int x=x0; x<=x1; ++x)
    {
      double v = _data[_ipt(x,y)];
      if (v == _missing)
      {
	printf("xxxxx ");
      }
      else
      {
	printf("%5.2f ", v);
      }
      if (++count > 15)
      {
	count = 0;
	printf("\n");
      }
    }
    printf("\n");
  }
}

//---------------------------------------------------------------------------
void Grid2d::printNonMissing(void) const
{
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      double v;
      if (getValue(x, y, v))
      {
	printf("[%d,%d]=%.5lf\n", x, y, v);
      }
    }
  }
}

//---------------------------------------------------------------------------
void Grid2d::debugShowMissing(void) const
{
  printf("Field:%s, missing=%.10lf    numMissing=%d\n",
	 _name.c_str(), _missing, _nx*_ny - numGood());
}

//---------------------------------------------------------------------------
bool Grid2d::getValue(int i, double &v) const
{
  if (_data[i] == _missing)
  {
    return false;
  }
  else
  {
    v = _data[i];
    return true;
  }
}

//---------------------------------------------------------------------------
bool Grid2d::getValue(int x, int y, double &v) const
{
  if (_data[_ipt(x, y)] == _missing)
  {
    return false;
  }
  else
  {
    v = _data[_ipt(x,y)];
    return true;
  }
}

//---------------------------------------------------------------------------
bool Grid2d::getValueAtOffset(int x, int y, int offset, double &v) const
{
  int ipt = _ipt(x,y) + offset;
  if (ipt < 0 || ipt >= _npt)
  {
    v = 0;
    return false;
  }
  else
  {
    return getValue(ipt, v);
  }
}

//---------------------------------------------------------------------------
double Grid2d::percentMissing(void) const
{
  double pcnt_bad=0.0;
  long numGood = 0;
  long numBad = 0;
  for(int k=0; k < _npt; k++)
  {
    if (isMissing(k))
    {
      numBad++;
    }
    else
    {
      numGood++;
    }
  }
    
  if (numGood == 0)
  {
    return 1.0;
  }
  else
  {
    pcnt_bad = static_cast<double>(numBad)/
      static_cast<double>(numGood + numBad);
    return pcnt_bad;
  }
}

//---------------------------------------------------------------------------
int Grid2d::numGood(void) const
{
  int nGood = 0;
  for(int k=0; k < _npt; k++)
  {
    if (_data[k] != _missing)
    {
      nGood++;
    }
  }
  return nGood;
}

//---------------------------------------------------------------------------
bool Grid2d::inRange(int x, int y) const
{
  return (x >= 0 && x < _nx && y >= 0 && y < _ny);
}

//---------------------------------------------------------------------------
bool Grid2d::equals(int x, int y, double value) const
{
  return _data[_ipt(x, y)] == value;
}

//---------------------------------------------------------------------------
bool Grid2d::greaterThan(int x, int y, double value) const
{
  return _data[_ipt(x, y)] > value;
}

//---------------------------------------------------------------------------
void Grid2d::setGridInfo(const std::vector<double> &d, double missing, int nx,
			 int ny)
{
  _data = d;
  _missing = missing;
  _npt = nx*ny;
  _nx = nx;
  _ny = ny;
}

//---------------------------------------------------------------------------
void Grid2d::setGridInfo(const std::vector<double> &d, int nx, int ny)
{
  _data = d;
  _npt = nx*ny;
  _nx = nx;
  _ny = ny;
}

//---------------------------------------------------------------------------
void Grid2d::setGridInfo(double missing)
{
  _missing = missing;
}

//---------------------------------------------------------------------------
void Grid2d::dataCopy(const Grid2d &g) 
{
  _data = g._data;
  _missing = g._missing;
  _npt = g._npt;
  _nx = g._nx;
  _ny = g._ny;
}

//---------------------------------------------------------------------------
void Grid2d::setAllMissing(void)
{
  for (size_t i=0; i<_data.size(); ++i)
  {
    _data[i] = _missing;
  }
}

//---------------------------------------------------------------------------
void Grid2d::setMissing(int ix, int iy)
{
  setValue(ix, iy, _missing);
}

//---------------------------------------------------------------------------
void Grid2d::setMissing(int i)
{
  setValue(i, _missing);
}

//---------------------------------------------------------------------------
void Grid2d::setMissingWithWarnings(int ix, int iy)
{
  setWithWarnings(ix, iy, _missing);
}

//---------------------------------------------------------------------------
void Grid2d::changeMissingAndData(double newmissing)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] == _missing)
    {
      _data[i] = newmissing;
    }
  }
  _missing = newmissing;
}

//---------------------------------------------------------------------------
void Grid2d::changeMissing(double newmissing)
{
  _missing = newmissing;
}

//---------------------------------------------------------------------------
bool Grid2d::setWithWarnings(int x, int y, double value)
{
  if (x < 0 || x >= _nx || y < 0 || y >= _ny)
  {
    printf("WARNING in Grid2d::set out of range got (%d,%d) range=(%d,%d)\n",
	   x, y, _nx, _ny);
    return false;
  }
  else
  {
    _data[_ipt(x, y)] = value;
    return true;
  }
}

//---------------------------------------------------------------------------
void Grid2d::setValue(const std::vector<std::pair<int,int> > &p, double value)
{
  std::vector<std::pair<int,int> >::const_iterator ip;
  for (ip=p.begin(); ip!=p.end(); ++ip)
  {
    setValue(ip->first, ip->second, value);
  }
}

//---------------------------------------------------------------------------
void Grid2d::setAllToValue(double value)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    _data[i] = value;
  }
}

//----------------------------------------------------------------
bool Grid2d::reduce(const int f)
{
  if (f < 2)
  {
    LOG(ERROR) << "cant reduce grid , factor too small " << f;
    return false;
  }
  int nx = _nx/f;
  int ny = _ny/f;
  if (nx < 1 || ny < 1)
  {
    LOG(ERROR) << "cant reduce grid, factor too big " << f;
    return false;
  }

  // make a grid for results
  Grid2d g(_name, nx, ny, _missing);

  // simply subsample the grid
  for (int y=0; y<ny; ++y)
  {
    int fullY = y*f;
    if (fullY >= _ny)
    {
      LOG(ERROR) << "fullY=" << fullY << " > _ny=" << _ny;
      continue;
    }
    for (int x=0; x<nx; ++x)
    {
      int fullX = x*f;
      if (fullX >= _nx)
      {
	LOG(ERROR) << "fullX= " << fullX << " > _nx=" << _nx;
	continue;
      }

      // store from fullX, fullY to grid at x,y
      g.setValue(x, y, _data[_ipt(fullX, fullY)]);
    }
  }
  *this = g;
  return true;
}

//----------------------------------------------------------------
bool Grid2d::interpolate(const Grid2d &lowres, const int res)
{
  if (lowres._nx != _nx/res)
  {
    LOG(ERROR) <<  "lowres nx= " << lowres._nx << " != _nx/res" << _nx/res;
    return false;
  }
  if (lowres._ny != _ny/res)
  {
    LOG(ERROR) <<  "lowres ny= " << lowres._ny << " != _ny/res" << _ny/res;
    return false;
  }

  // loop through high res grid, compute low res point 
  for (int y=0; y<_ny; ++y)
  {
    int ry0 = y/res;
    for (int x=0; x<_nx; ++x)
    {
      int rx0 = x/res;

      // do bilinear interpolation 
      double v = _bilinear(ry0, rx0, res, y, x, lowres);

      // store to high res (local) grid
      setValue(x, y, v);
    }
  }
  return true;
}



int Grid2d::_firstValidIndex(int y) const
{
  for (int x=0; x<_nx; ++x)
  {
    if (_data[_ipt(x, y)] != _missing)
    {
      return x;
    }
  }
  return -1;
}


int Grid2d::_lastValidIndex(int y) const
{
  for (int x=_nx-1; x>=0; --x)
  {
    if (_data[_ipt(x, y)] != _missing)
    {
      return x;
    }
  }
  return -1;
}


//----------------------------------------------------------------
double Grid2d::_bilinear(int ry0, int rx0, int res, int y, int x,
			 const Grid2d &lowres) const
{
  int x0 = rx0*res;
  int y0 = ry0*res;
  int x1 = x0 + res;
  int y1 = y0 + res;
  double f00=_missing, f01=_missing, f10=_missing, f11=_missing;
  int rx1 = rx0 + 1;
  int ry1 = ry0 + 1;
  bool ok = true;
  if (lowres.inRange(rx0, ry0))
  {
    if (!lowres.getValue(rx0, ry0, f00))
    {
      f00 = _missing;
      ok = false;
    }
  }
  if (lowres.inRange(rx0, ry1))
  {
    if (!lowres.getValue(rx0, ry1, f01))
    {
      f01 = _missing;
      ok = false;
    }
  }
  if (lowres.inRange(rx1, ry0))
  {
    if (!lowres.getValue(rx1, ry0, f10))
    {
      f10 = _missing;
      ok = false;
    }
  }
  if (lowres.inRange(rx1, ry1))
  {
    if (!lowres.getValue(rx1, ry1, f11))
    {
      f11 = _missing;
      ok = false;
    }
  }

  if (!ok)
  {
    return _missing;
  }
  else
  {
    double ret = (f00*(x1-x)*(y1-y) +
		  f10*(x-x0)*(y1-y) +
		  f01*(x1-x)*(y-y0) +
		  f11*(x-x0)*(y-y0));
    ret = ret/((x1-x0)*(y1-y0));
    return ret;
  }
}

