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
 * @file GridAlgs.c
 */

#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dLoopAlg.hh>
#include <euclid/Grid2dOffset.hh>
#include <euclid/Grid2dLoop.hh>
#include <euclid/Grid2dLoopA.hh>
#include <euclid/Grid2dMedian.hh>
#include <euclid/Line.hh>
#include <euclid/PointList.hh>
#include <rapmath/AngleCombiner.hh>
#include <rapmath/FuzzyF.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <sstream>
using std::vector;
using std::pair;
using std::stringstream;

//-----------------------------------------------------------------
static void _interp(double d0, double d1, int i0, int i1,
		    std::vector<double> &iData)
{
  // i0 is index of first data, i1 of second data
  // so i0+1, i0+2,... i1-1 are the places to interpolate to
  int nx = i1-i0;
  for (int i=1; i<nx; ++i)
  {
    int index = i0 + i;
    double pct = (double)i/(double)nx;
    double v = (1.0-pct)*d0 + pct*d1;
    iData[index] = v;
    LOG(DEBUG_VERBOSE) << "interp data[" << index << "] = " << v;
  }
}

//-----------------------------------------------------------------
static void _fillGaps(std::vector<double> &data, double missing)
{
  int n = static_cast<int>(data.size());

  bool inside = true;
  int o0 = -1, o1 = -1;
  for (int i=0; i<n; ++i)
  {
    if (data[i] == missing)
    {
      if (inside)
      {
	inside = false;
	o0 = o1 = i;
	LOG(DEBUG_VERBOSE) << "First point missing index=" << i;
      }
      else
      {
	o1 = i;
      }
    }
    else
    {
      if (!inside)
      {
	// went from outside to inside, now can interp
	inside = true;
	int interp0 = o0 - 1;
	int interp1 = i;
	LOG(DEBUG_VERBOSE) << "First point leaving missing index="
			   << i << ", i0=" << interp0 << " i1=" << interp1;
	if (interp0 >= 0)
	{
	  _interp(data[interp0], data[interp1], interp0, interp1, data);
	}
      }
    }
  }
}

/*----------------------------------------------------------------*/
static double _round(double v, double r0, double r1, double res)
{
  int index;

  if (v <= r0)
    return r0;
  if (v >= r1)
    return r1;

  index = (int)((v - r0)/res + 0.5);
  return r0 + (double)index*res;
}

//----------------------------------------------------------------
static double _median2(Grid2d &out, int xw, int yw, Grid2dMedian &F,
		       Grid2dLoop &G)
{
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // update Grid2dMedian object
  F.update(newv, oldv);
  return F.getMedian();
}

// //----------------------------------------------------------------
// static double _speckle(Grid2d &out, int xw, int yw, Grid2dMedian &F,
// 		       Grid2dLoop &G) 
// {
//   vector<pair<int,int> >  newv, oldv;
//   vector<pair<int,int> >::iterator i;

//   // get old and new points based on state
//   newv = G.newXy(xw, yw);
//   oldv = G.oldXy(xw, yw);
  
//   // update Grid2dMedian object
//   F.update(newv, oldv);

//   double p25, p75;
//   p25 = F.getPercentile(0.25);
//   p75 = F.getPercentile(0.75);
//   if (p25 == F.getMissing() || p75 == F.getMissing())
//   {
//     return out.getMissing();
//   }
//   else
//   {
//     return p75 - p25;
//   }
// }

// //----------------------------------------------------------------
// static double _speckleInterest(Grid2d &out, int xw, int yw,
// 			       const FuzzyF &fuzzyDataDiff,
// 			       const FuzzyF &fuzzyCountPctDiff,
// 			       Grid2dMedian &F, Grid2dLoop &G) 
// {
//   vector<pair<int,int> >  newv, oldv;
//   vector<pair<int,int> >::iterator i;

//   // get old and new points based on state
//   newv = G.newXy(xw, yw);
//   oldv = G.oldXy(xw, yw);
  
//   // update Grid2dMedian object
//   F.update(newv, oldv);

//   double p25, p50, p75;
//   p25 = F.getPercentile(0.25);
//   p50 = F.getPercentile(0.50);
//   p75 = F.getPercentile(0.75);

//   double c25, c50, c75;
//   c25 = F.getCount(0.25);
//   c50 = F.getCount(0.50);
//   c75 = F.getCount(0.75);
//   double missing = F.getMissing();

//   if (p25 == missing || p50 == missing || p75 == missing ||
//       c25 == missing || c50 == missing || c75 == missing)
//   {
//     return out.getMissing();
//   }
//   else
//   {
//     double interest = 1.0;
//     interest *= fuzzyDataDiff.apply(p50-p25);
//     interest *= fuzzyDataDiff.apply(p75-p50);
    
//     double num = F.getNum();
//     c25 /= num;
//     c50 /= num;
//     c75 /= num;

//     interest *= fuzzyCountPctDiff.apply(fabs(c50-c25));
//     interest *= fuzzyCountPctDiff.apply(fabs(c75-c50));
    
//     return interest;
//   }
// }

//----------------------------------------------------------------
static double _medianInBox(Grid2d &data, int x0, int y0, int nx, int ny, 
			   bool allow_any_data, Grid2dMedian &F)
{
  F.clear();
  
  for (int y=y0; y<y0+ny; ++y)
  {
    for (int x=x0; x<x0+nx; ++x)
    {
      if (data.inRange(x, y))
      {
	double v;
	if (data.getValue(x, y, v))
	{
	  F.addValue(v);
	}
      }
    }
  }
  if (allow_any_data)
  {
    return F.getMedianAllData();
  }
  else
  {
    return F.getMedian();
  }
}


//------------------------------------------------------------------
TaThread *GridAlgs::GridAlgThreads::clone(int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(GridAlgs::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(void) : Grid2d()
{
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(const std::string &name)  : Grid2d(name)
{
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(const std::string &name, double missing) :
  Grid2d(name, missing)
{
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(const std::string &name, int nx, int ny, double missing) :
  Grid2d(name, nx, ny, missing)
{
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(const std::string &name, int nx, int ny, 
		   const std::vector<double> &data, double missing) :
  Grid2d(name, nx, ny, data, missing)
{
}

//---------------------------------------------------------------------------
GridAlgs::GridAlgs(const Grid2d &g)  : Grid2d(g)
{
}

//---------------------------------------------------------------------------
GridAlgs::~GridAlgs(void)
{
}

//---------------------------------------------------------------------------
GridAlgs GridAlgs::promote(const Grid2d &g)
{
  return GridAlgs(g._name, g._nx, g._ny, g._data, g._missing);
}

//---------------------------------------------------------------------------
double GridAlgs::evaluateData(bool debug) const
{
  int first = 1;
  double min=0.0, max=0.0, mean, total=0.0, pcnt_bad=0.0;
  long numGood = 0;
  long numBad = 0;
  double newBad, d;
  for(int k=0; k < _npt; k++)
  {
    d = _data[k];
    if (d == _missing)
    {
      numBad++;
    }
    else
    {
      numGood++;
      total = total + d;
      if (first)
      {
	min = max = d;
	first = 0;
      }
      else
      {
	if (d < min)
	{
	  min = d;
	}
	if (d > max)
	{
	  max = d;
	}
      }
    }
  }
    
  if (numGood == 0)
  {
    if (debug)
    {
      printf("%s all output data missing..unchanged\n", _name.c_str());
    }
    return _missing;
  }
  else
  {
    mean = total / double(numGood);
    pcnt_bad = static_cast<double>(numBad)/
      static_cast<double>(numGood + numBad);
    newBad = max + 1.0;
    if (debug)
    {
      printf("%s:npt:%d min,max=[%.2lf,%.2lf], mean=%.2lf,pct_bad=%.2lf\n",
	     _name.c_str(), _npt, min, max, mean, pcnt_bad);
    }
    return newBad;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::percentGreaterOrEqual(double v0) const
{
  long numBelow = 0;
  long numAbove = 0;
  for(int k=0; k < _npt; k++)
  {
    double d = _data[k];
    if (d != _missing)
    {
      if (d >= v0)
      {
	numAbove++;
      }
      else
      {
	numBelow++;
      }
    }	    
  }
    
  if (numAbove == 0 && numBelow == 0)
  {
    return 0.0;
  }
  else
  {
    double pcnt = static_cast<double>(numAbove)/
      static_cast<double>(numAbove + numBelow);
    return pcnt;
  }
}

//---------------------------------------------------------------------------
std::vector<double> GridAlgs::listValues(int max) const
{
  std::vector<double> v;
  double d;
  for(int k=0; k < _npt; k++)
  {
    d = _data[k];
    if (d != _missing)
    {
      if (find(v.begin(), v.end(), d) == v.end())
      {
	v.push_back(d);
	if (static_cast<int>(v.size()) > max)
	{
	  printf("WARNING more than %d values in %s\n",
		 max, _name.c_str());
	  return v;
	}
      }
    }
  }
  return v;
}

//---------------------------------------------------------------------------
std::vector<std::pair<int,int> > GridAlgs::pointsAtValue(double value) const
{
  std::vector<std::pair<int,int > > v;
  int ipt;
  for(int y=0; y < _ny; y++)
  {
    ipt = _ipt(0, y);
    for(int x=0; x < _nx; x++)
    {
      if (_data[ipt+x] == value)
      {
	v.push_back(std::pair<int,int>(x,y));
      }
    }
  }
  return v;
}

//---------------------------------------------------------------------------
bool GridAlgs::isEdge(int x0, int y0) const
{
  // IS an edge if on edge of data (designers choice.)
  if (x0 < 1 || x0 >= _nx-2)
  {
    return true;
  }
  if (y0 < 1 || y0 >= _ny-2)
  {
    return true;
  }
  
  int ipt0 = _ipt(x0,y0);
  if (_data[ipt0-1] == _missing)
  {
    return true;
  }
  if (_data[ipt0+1] == _missing)
  {
    return true;
  }
  if (_data[ipt0+_nx] == _missing)
  {
    return true;
  }
  if (_data[ipt0-_nx] == _missing)
  {
    return true;
  }
  if (_data[ipt0 + _nx - 1] == _missing)
  {
    return true;
  }
  if (_data[ipt0 + _nx + 1] == _missing)
  {
    return true;
  }
  if (_data[ipt0 - _nx - 1] == _missing)
  {
    return true;
  }
  if (_data[ipt0 - _nx + 1] == _missing)
  {
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
bool GridAlgs::isValidWithAtMostOneNeighbor(int ix, int iy) const
{
  if (isMissing(ix, iy))
  {
    return false;
  }
  int ndiag = 0;
  int nadj = 0;
  for (int y=iy-1; y<= iy+1; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=ix-1; x<=ix+1; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      if (x == ix && y == iy)
      {
	continue;
      }
      if (!isMissing(x, y))
      {
	if (x == ix || y == iy)
	{
	  ++nadj;
	}
	else
	{
	  ++ndiag;
	}
	if (nadj > 1)
	{
	  return false;
	}
      }
    }
  }
  return (nadj == 1 || (nadj == 0 && ndiag <= 1));
}

//---------------------------------------------------------------------------
bool GridAlgs::isHoleSingle(int ix, int iy, double &v) const
{
  if (ix < 1 || ix >= _nx -2)
  {
    return false;
  }
  if (iy < 1 || iy >= _ny -2)
  {
    return false;
  }
  if (!isMissing(ix, iy))
  {
    return false;
  }
  if (!isMissing(ix-1, iy-1) &&
      !isMissing(ix,   iy-1) &&
      !isMissing(ix+1, iy-1) &&
      !isMissing(ix-1, iy) &&
      !isMissing(ix+1, iy))
  {
    v = _data[iy*_nx + ix-1];
    return true;
  }
  if (!isMissing(ix-1, iy+1) &&
      !isMissing(ix,   iy+1) &&
      !isMissing(ix+1, iy+1) &&
      !isMissing(ix-1, iy) &&
      !isMissing(ix+1, iy))
  {
    v = _data[iy*_nx + ix-1];
    return true;
  }
  if (!isMissing(ix+1, iy-1) &&
      !isMissing(ix+1, iy) &&
      !isMissing(ix+1, iy+1) &&
      !isMissing(ix,   iy-1) &&
      !isMissing(ix,   iy+1))
  {
    v = _data[iy*_nx + ix+1];
    return true;
  }
  if (!isMissing(ix-1, iy-1) &&
      !isMissing(ix-1, iy) &&
      !isMissing(ix-1, iy+1) &&
      !isMissing(ix,   iy-1) &&
      !isMissing(ix,   iy+1))
  {
    v = _data[iy*_nx + ix-1];
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
bool GridAlgs::minMax(double &minv, double &maxv) const
{
  bool first = true;
  double d;
  for(int k=0; k < _npt; k++)
  {
    d = _data[k];
    if (d == _missing)
    {
      continue;
    }
    if (first)
    {
      first = false;
      minv = maxv = d;
    }
    else
    {
      if (d < minv)
      {
	minv = d;
      }
      if (d > maxv)
      {
	maxv = d;
      }
    }
  }
  return !first;
}

//---------------------------------------------------------------------------
bool GridAlgs::boundingBox(int &x0, int &x1, int &y0, int &y1) const
{
  bool first = true;
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      if (!isMissing(x, y))
      {
	if (first)
	{
	  first = false;
	  x0 = x1 = x;
	  y0 = y1 = y;
	}
	else
	{
	  if (x < x0)
	  {
	    x0 = x;
	  }
	  if (x > x1)
	  {
	    x1 = x;
	  }
	  if (y < x0)
	  {
	    y0 = y;
	  }
	  if (y > y1)
	  {
	    y1 = y;
	  }
	}
      }
    }
  }

  return !first;
}

//---------------------------------------------------------------------------
std::string GridAlgs::getInfoForAlg(bool debug, double &missing,
				    double &newMissing) const
{
  missing = _missing;
  newMissing = evaluateData(debug);
  return _name;
}

//---------------------------------------------------------------------------
std::string GridAlgs::getInfoForAlg(double &missing) const
{
  missing = _missing;
  return _name.c_str();
}

//---------------------------------------------------------------------------
bool GridAlgs::intersects(const Grid2d &g, double &quality) const
{
  if (_nx != g._nx || _ny != g._ny)
  {
    printf("EROR in comparing grids uneven dimensions\n");
    return false;
  }
  int n0=0, n1=0, ngood0=0, ngood1=0;
  bool stat = false;
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (intersects(i, g, ngood0, n0, ngood1, n1))
    {
      stat = true;
    }
  }
  if (stat) 
  {
    double q0, q1;
    q0 = static_cast<double>(ngood0)/static_cast<double>(n0);
    q1 = static_cast<double>(ngood1)/static_cast<double>(n1);
    if (q0 > q1)
    {
      quality = q0;
    }
    else
    {
      quality = q1;
    }
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
bool GridAlgs::intersects(int i, const Grid2d &g, int &ngood0, int &n0,
			  int &ngood1, int &n1) const
{
  bool g0, g1;
  g0 = !isMissing(i);
  g1 = !g.isMissing(i);
  if (g0)
  {
    ++n0;
  }
  if (g1)
  {
    ++n1;
  }
  if (g0 && g1)
  {
    ++ngood0;
    ++ngood1;
    return true;
  }
  else
  {
    return false;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::max(const Grid2d &g)
{
  if (_nx != g._nx || _ny != g._ny)
  {
    printf("ERROR in grid max, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v2;
    if (g.getValue(i, v2))
    {
      double v;
      if (getValue(i, v))
      {
	if (v2 > v)
	{
	  setValue(i, v2);
	}
      }
      else
      {
	setValue(i, v2);
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::min(const Grid2d &g)
{
  if (_nx != g._nx || _ny != g._ny)
  {
    printf("ERROR in grid min, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v2;
    if (g.getValue(i, v2))
    {
      double v;
      if (getValue(i, v))
      {
	if (v2 < v)
	{
	  setValue(i, v2);
	}
      }
      else
      {
	setValue(i, v2);
      }
    }
  }
}

//---------------------------------------------------------------------------
double GridAlgs::localMax(int ix, int iy, int sx,  int sy) const
{
  double vt = 0.0;
  double v = 0.0;
  bool first=true;
  for (int y=iy-sy; y<=iy+sy; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=ix-sx; x<=ix+sx; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      if (getValue(x, y, v))
      {
	if (first)
	{
	  first = false;
	  vt = v;
	}
	else
	{
	  if (v > vt)
	  {
	    vt = v;
	  }
	}
      }
    }
  }
  if (first)
  {
    return _missing;
  }
  else
  {
    return vt;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::averageAtX(int ix) const
{
  if (ix < 0 || ix >= _nx)
  {
    return _missing;
  }
  double vt=0;
  double v;
  double n=0;
  for (int y=0; y<_ny; ++y)
  {
    if (getValue(ix, y, v))
    {
      vt += v;
      n++;
    }
  }
  if (n==0)
  {
    return _missing;
  }
  else
  {
    return vt/n;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::maxAtX(int ix) const
{
  if (ix < 0 || ix >= _nx)
  {
    return _missing;
  }
  double max=0;
  double v;
  bool first = true;
  for (int y=0; y<_ny; ++y)
  {
    if (getValue(ix, y, v))
    {
      if (first)
      {
	first = false;
	max = v;
      }
      else
      {
	if (v > max)
	{
	  max = v;
	}
      }
    }
  }
  if (first)
  {
    return _missing;
  }
  else
  {
    return max;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::adjust(int x0, int x1)
{
  if (x0 <= 0 && (x1 >= _nx-1 || x1 < 0))
  {
    return;
  }

  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<x0; ++x)
    {
      _data[_ipt(x,y)] = _missing;
    }
    if (x1 > 0)
    {
      for (int x=x1+1; x<_nx; ++x)
      {
	_data[_ipt(x,y)] = _missing;
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::increment(int x, int y, double value)
{
  double v;
  if (getValue(x, y, v))
  {
    _data[_ipt(x,y)] = v +  value;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::increment(int ipt, double value)
{
  double v;
  if (getValue(ipt, v))
  {
    _data[ipt] = v +  value;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::add(double value)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v;
    if (getValue(i, v))
    {
      setValue(i, v+value);
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::add(const Grid2d &g, Grid2d &count)
{
  if (_nx != g._nx || _ny != g._ny ||
      _nx != count._nx || _ny != count._ny)
  {
    printf("ERROR in grid add, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v2;
    if (g.getValue(i, v2))
    {
      double v;
      if (getValue(i, v))
      {
	setValue(i, v+v2);
      }
      else
      {
	setValue(i, v2);
      }
      double c;
      if (count.getValue(i, c))
      {
	count.setValue(i, c+1);
      }
      else
      {
	count.setValue(i, 1);
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::add(const Grid2d &g)
{
  if (_nx != g._nx || _ny != g._ny)
  {
    printf("ERROR in grid add, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v2;
    if (g.getValue(i, v2))
    {
      double v;
      if (getValue(i, v))
      {
	setValue(i, v+v2);
      }
      else
      {
	setValue(i, v2);
      }
    }
  }
}


//---------------------------------------------------------------------------
void GridAlgs::multiply(int x, int y, double value)
{
  double v;
  if (getValue(x, y, v))
  {
    _data[_ipt(x,y)] = v*value;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::multiply(const Grid2d &g)
{
  if (_nx != g._nx || _ny != g._ny)
  {
    printf("ERROR in grid product, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v;
    if (getValue(i, v))
    {
      double v2;
      if (g.getValue(i, v2))
      {
	setValue(i, v*v2);
      }
      else
      {
	setValue(i, _missing);
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::multiply(double value)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v;
    if (getValue(i, v))
    {
      setValue(i, v*value);
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::divide(const Grid2d &div)
{
  if (_nx != div._nx || _ny != div._ny )
  {
    printf("ERROR in grid div, dims unequal\n");
    return;
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    double v2;
    if (div.getValue(i, v2))
    {
      if (v2 == 0)
      {
	continue;
      }
      double v;
      if (getValue(i, v))
      {
	setValue(i, v/v2);
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::change(double oldvalue, double newvalue)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (_data[i] == oldvalue)
    {
      _data[i] = newvalue;
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::changeLessOrEqual(double oldvalue, double newvalue)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (_data[i] == _missing)
    {
      _data[i] = newvalue;
    }
    else
    {
      if (_data[i] <= oldvalue)
      {
	_data[i] = newvalue;
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::change(std::vector<double> oldvalue, double skipvalue,
		      double newvalue)
{
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (_data[i] == skipvalue)
    {
      continue;
    }
    vector<double>::const_iterator ii;
    if (find(oldvalue.begin(), oldvalue.end(), _data[i]) != oldvalue.end())
    {
      _data[i] = newvalue;
    }
  }
}

//---------------------------------------------------------------------------
bool GridAlgs::merge(const Grid2d &g)
{
  if (g._nx != _nx || g._ny != _ny)
  {
    printf("ERROR merging grids, dimensions unequal\n");
    return false;
  }
  double v;
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (g.getValue(i, v))
    {
      _data[i] = v;
    }
  }
  return true;
}

//---------------------------------------------------------------------------
bool GridAlgs::weightedCentroid(double maskvalue, const Grid2d &weights,
				int &x, int &y) const
{
  if (_nx != weights._nx || _ny != weights._ny)
  {
    return false;
  }
  double n=0.0, w=0.0, v;
  double fx=0.0, fy=0.0;
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      if (_data[_ipt(ix, iy)] == maskvalue)
      {
	v = weights._data[_ipt(ix, iy)];
	if (v == weights._missing)
	{
	  continue;
	}
	++n;
	w += v;
	fx += static_cast<double>(ix)*v;
	fy += static_cast<double>(iy)*v;
      }
    }
  }
  if (n == 0.0)
  {
    return false;
  }
  if (w == 0.0)
  {
    return false;
  }
  x = static_cast<int>(fx/w);
  y = static_cast<int>(fy/w);
  return true;
}

/*------------------------------------------------------------------------*/
bool GridAlgs::angleAverageInMask(const Grid2d &mask, double &a) const
{
  AngleCombiner f(mask.numGood(), true);
  f.clearValues();
  int count = 0;

  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double ai;
    if (getValue(i, ai))
    {
      f.setGood(count++, ai, 1.0);
    }
  }
  return f.getCombineAngle(a);
}

/*----------------------------------------------------------------*/
bool GridAlgs::averageInMask(const Grid2d &mask, double &a) const
{
  double count = 0;
  double ave = 0;

  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double ai;
    if (getValue(i, ai))
    {
      ave += ai;
      count += 1;
    }
  }
  if (count > 0)
  {
    a = ave/count;
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
double GridAlgs::maxInMask(const Grid2d &mask) const
{
  bool first = true;
  double max = 0;

  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double ai;
    if (getValue(i, ai))
    {
      if (first)
      {
	first = false;
	max = ai;
      }
      else
      {
	if (ai > max)
	{
	  max = ai;
	}
      }
    }
  }
  if (first)
  {
    return _missing;
  }
  else
  {
    return max;
  }
}


/*----------------------------------------------------------------*/
bool GridAlgs::largestAverageExceedingThreshInMask(const Grid2d &mask,
						   double radius,
						   double threshold,
						   int &x0, int &y0) const
{
  x0 = y0 = -1;

  int dmax, d;
  bool first;

  // build a list of offsets all points inside box out to radius.
  Grid2dOffset off(radius, 0.0, _nx, _missing);
    
  // now find all points within percent of max and
  // count nearby points within percent of max.
  first = true;
  x0 = y0 = -1;
  dmax = _missing;
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      double v, m;
      if (!getValue(x,y, v) || !mask.getValue(x,y, m))
      {
	continue;
      }
      if (v < threshold)
      {
	continue;
      }
      d = off.numMaskedExceeding(*this, x, y, threshold, mask);
      if (first)
      {
	first = false;
	dmax = d;
	x0 = x;
	y0 = y;
      }
      else
      {
	if (d > dmax)
	{
	  dmax = d;
	  x0 = x;
	  y0 = y;
	}
      }
    }
  }
  return (x0 != -1 && y0 != -1);
}

/*----------------------------------------------------------------*/
bool GridAlgs::rangeInMask(const Grid2d &mask, double &v0, double &v1) const
{
  bool first = true;
  v0 = v1 = _missing;
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }

    double v;
    if (!getValue(i, v))
    {
      continue;
    }
    if (first)
    {
      first = false;
      v0 = v1 = v;
    }
    else
    {
      if (v > v1)
	v1 = v;
      if (v < v0)
	v0 = v;
    }
  }
  return (!first);
}

/*----------------------------------------------------------------*/
bool GridAlgs::angleDifferenceInMask(const Grid2d &mask,
				     const Grid2d &other,
				     double &aveDiff) const
{
  double d, nsum;
    
  // for each point, see if its non-missing and in some region.
  aveDiff = nsum = 0.0;
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double a, aother;
    if (!getValue(i, a) || !other.getValue(i, aother))
    {
      continue;
    }
	    
    // now actually compare, looking for minimum diff.
    d = a - aother;
    d = fabs(d);
    while (d > 180.0)
    {
      d -= 180.0;
    }
    if (d > 90.0)
    {
      d = 180.0 - d;
    }
    aveDiff += d;
    nsum ++;
  }

  if (nsum > 0)
  {
    aveDiff = aveDiff/nsum;
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
double GridAlgs::percentInMask(const Grid2d &mask) const
{
  double nm, no;
    
  nm = no = 0.0;
    
  for (int i=0; i<_npt; ++i)
  {
    if (isMissing(i))
    {
      continue;
    }
    if (mask.isMissing(i))
    {
      no++;
    }
    else
    {
      nm++;
    }
  }
  if (nm + no == 0)
  {
    return 0.0;
  }
  else
  {
    return nm/(nm + no);
  }
}

/*----------------------------------------------------------------*/
bool GridAlgs::motionDifferenceInMask(const Grid2d &mask,
				      const Grid2d &other,
				      double &motionDiff) const
{
  double d, nsum;
    
  // for each point, see if its non-missing and in some region.
  motionDiff = nsum = 0.0;
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double a, aother;
    if (!getValue(i, a) || !other.getValue(i, aother))
    {
      continue;
    }
    // now actually compare, looking for minimum diff.
    d = a - aother;
    d = fabs(d);
    while (d >= 360.0)
    {
      d -= 360.0;
    }
    if (d > 180.0)
    {
      d = 360.0 - d;
    }
    motionDiff += d;
    nsum ++;
  }

  if (nsum > 0)
  {
    motionDiff = motionDiff/nsum;
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
void GridAlgs::maskRange(const Grid2d &mask, double low, double high)
{
  if (mask._nx != _nx || mask._ny != _ny)
  {
    LOG(ERROR) << "unequal grid dim input("
	       << mask._nx << "," << mask._ny << ") local("
	       << _nx << "," << _ny << ")";
    return;
  }
  for (int i=0; i<mask._npt; ++i)
  {
    double v;
    if (mask.getValue(i, v))
    {
      if (v >= low && v <= high)
      {
	setValue(i, _missing);
      }
    }
    else
    {
      setValue(i, _missing);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::rescaleInMask(const Grid2d &mask, double scale)
{
  double v;
    
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i) || !getValue(i, v))
    {
      continue;
    }
    v = v*scale;
    setValue(i, v);
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::fillInMask(const Grid2d &mask, double value)
{
  for (int i=0; i<_npt; ++i)
  {
    if (!mask.isMissing(i))
    {
      setValue(i, value);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::incrementInMask(const Grid2d &mask, double inc)
{
  for (int i=0; i<_npt; ++i)
  {
    if (!mask.isMissing(i))
    {
      increment(i, inc);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::maskMissingToMissing(const Grid2d &mask)
{
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      setMissing(i);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::fillInMask(const Grid2d &mask)
{
  for (int i=0; i<_npt; ++i)
  {
    double v;
    if (mask.getValue(i, v))
    {
      setValue(i, v);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::clearMaskWhereMissingWhileSetting(Grid2d &mask, double v)
{
  for (int i=0; i<_npt; ++i)
  {
    if (isMissing(i) && !mask.isMissing(i))
    {
      mask.setMissing(i);
      setValue(i, v);
    }
  }
}

/*----------------------------------------------------------------*/
#define MAX_RADIUS 20
void GridAlgs::expandInMask(const Grid2d &mask)
{
  int x, y, r;
  vector<int> olist;
  vector<int>::iterator it;

  for (y=0; y<_ny; ++y)
  {
    for (x=0; x<_nx; ++x)
    {
      // look for not missing in mask and missing in this.
      if ((!isMissing(x, y)) || mask.isMissing(x,y))
      {
	continue;
      }
      
      // look for nearest non-missing point in this.
      for (r=1; r<MAX_RADIUS; ++r)
      {
	olist = _orderedIndices(x, y, r);
	if (olist.empty())
	{
	  // radius too big..outside data.
	  break;
	}
	for (it=olist.begin(); it != olist.end(); ++it)
	{
	  double d;
	  if (getValue(*it, d))
	  {
	    setValue(x, y, d);
	    break;
	  }
	}
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::intersection(const Grid2d &mask)
{
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      setMissing(i);
    }
  }
}

/*----------------------------------------------------------------*/
#define MAX_RADIUS 20
void GridAlgs::nearestSameWithMaskSame(const Grid2d &data,
				      const Grid2d &conf,
				      int x, int y,
				      const Grid2d &mask,
				      double mask_value,
				      Grid2d &out_conf)
{
  int r;
  vector<int>::iterator it;
    
  // go out to bigger and bigger radii till get a match.
  for (r=1; r<MAX_RADIUS; ++r)
  {
    vector<int> olist = _orderedIndices(x, y, r);
    if (olist.empty())
      break;

    for (it=olist.begin(); it != olist.end(); ++it)
    {
      if (mask.getDataAt(*it) != mask_value)
      {
	// wrong region.
	continue;
      }

      double v, c;
      if (!data.getValue(*it, v) || !conf.getValue(*it, c))
      {
	// nothing good to copy in.
	continue;
      }

      // this is it.
      setValue(x, y, v);
      out_conf.setValue(x, y, c);
      return;
    }
  }
  setMissing(x, y);
  out_conf.setMissing(x, y);
}

//----------------------------------------------------------------
void GridAlgs::setMaskToMissing(const Grid2d &mask)
{
  for (int i=0;i<_npt; ++i)
  {
    if (!mask.isMissing(i))
    {
      setMissing(i);
    }
  }
}

//----------------------------------------------------------------
bool GridAlgs::orientationAngleAverageInMask(const Grid2d &mask,
					    double &ave) const
{
  AngleCombiner A(mask.numGood());
  A.clearValues();
  int count=0;
  for (int i=0; i<_npt; ++i)
  {
    if (mask.isMissing(i))
    {
      continue;
    }
    double a;
    if (getValue(i, a))
    {
      A.setGood(count++, a, 1.0);
    }
  }
  return A.getCombineAngle(ave);
}

//---------------------------------------------------------------------------
double GridAlgs::localCenteredAverage(int ix, int iy, int sx, int sy,
				      bool needHalf) const
{
  double vt=0;
  double v;
  double n=0;
  for (int y=iy-sy; y<=iy+sy; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=ix-sx; x<=ix+sx; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      if (getValue(x, y, v))
      {
	vt += v;
	n++;
      }
    }
  }
  double maxbad;
  if (needHalf)
  {
    // slightly relaxed
    maxbad = static_cast<double>((sx-1)*(sy-1))/2.0;
  }
  else
  {
    maxbad = 0;
  }

  if (n > maxbad)
  {
    return vt/n;
  }
  else
  {
    return _missing;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::localBoxAverage(int x0, int y0, int nx, int ny) const
{
  double vt=0;
  double n=0;
  for (int y=y0; y<y0+ny; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=x0; x<x0+nx; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      double v;
      if (getValue(x, y, v))
      {
	vt += v;
	n++;
      }
    }
  }
  if (n > 0)
  {
    return vt/n;
  }
  else
  {
    return _missing;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::localMeanXy(int xLwr, int xUpr, int yLwr, int yUpr) const
{
  double vt=0;
  double n=0;
  for (int y=yLwr; y<=yUpr; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=xLwr; x<=xUpr; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      double v;
      if (getValue(x, y, v))
      {
	vt += v;
	n++;
      }
    }
  }
  if (n > 0)
  {
    return vt/n;
  }
  else
  {
    return _missing;
  }
}

//---------------------------------------------------------------------------
double GridAlgs::localCenteredAverageNoMissing(int ix, int iy, int sx,
					       int sy) const
{
  double vt=0;
  double v;
  double n=0;
  for (int y=iy-sy; y<=iy+sy; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=ix-sx; x<=ix+sx; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      if (getValue(x, y, v))
      {
	vt += v;
	n++;
      }
      else
      {
	return _missing;
      }
    }
  }
  if (n==0)
  {
    return _missing;
  }
  else
  {
    return vt/n;
  }
}

//----------------------------------------------------------------
void GridAlgs::smooth(int xw, int yw)
{
  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);

  // make a copy of the local object
  GridAlgs tmp(*this);

  Grid2dLoopAlgMean A;

  while (G.increment(tmp, A))
  {
    int x, y;
    double result;
    if (G.getXyAndResult(A, xw*yw/2, x, y, result))
    {
      _data[y*_nx + x] = result;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}


//---------------------------------------------------------------------------
void GridAlgs::smoothSimple(int sx, int sy)
{
  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      double v = localCenteredAverage(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::smoothThreaded(int sx, int sy, int numThread)
{
  GridAlgThreads *thread = new GridAlgs::GridAlgThreads();
  thread->init(numThread, false);

  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::SMOOTH,
					  sx, sy, iy, this, tmp);
    thread->thread(iy, (void *)info);
  }
  thread->waitForThreads();
  delete thread;
  *this = tmp;
}


//----------------------------------------------------------------
void GridAlgs::smoothNoMissing(int xw, int yw)
{
  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);

  // make a copy of the local object
  GridAlgs tmp(*this);

  Grid2dLoopAlgMeanNoMissing A;

  while (G.increment(tmp, A))
  {
    int x, y;
    double result;
    if (G.getXyAndResult(A, xw*yw/2, x, y, result))
    {
      _data[y*_nx + x] = result;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::smoothNoMissingSimple(int sx, int sy)
{
  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      double v = localCenteredAverageNoMissing(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::fillGaps(int sx, int sy)
{
  GridAlgs tmp(*this);
  double v;
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      if (getValue(ix, iy, v))
      {
	continue;
      }
      double v = localCenteredAverage(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
double GridAlgs::localSdevXy(int xLwr, int xUpr, int yLwr, int yUpr) const
{
  double N=0.0, Q = 0.0, A = 0.0;

  for (int y=yLwr; y<=yUpr; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=xLwr; x<=xUpr; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      double v;
      if (getValue(x, y, v))
      {
        N+=1;
        Q = Q+((N-1)/N)*(v-A)*(v-A);
        A = A+(v-A)/N;
      }
    }
  }
  if (N > 0)
  {
    return sqrt(Q/N);
  }
  else
  {
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::localBoxSdev(int x0, int y0, int nx, int ny,
			      bool needHalf) const
{
  double N=0.0, Q = 0.0, A = 0.0;

  for (int y=y0; y<y0+ny; ++y)
  {
    if (y < 0 || y >= _ny)
    {
      continue;
    }
    for (int x=x0; x<x0+nx; ++x)
    {
      if (x < 0 || x >= _nx)
      {
	continue;
      }
      double v;
      if (getValue(x, y, v))
      {
        N+=1;
        Q = Q+((N-1)/N)*(v-A)*(v-A);
        A = A+(v-A)/N;
      }
    }
  }

  double maxbad;
  if (needHalf)
  {
    // did this to agree with other approaches, slightly forgiving
    maxbad = static_cast<double>((nx-1)*(ny-1))/2.0;
  }
  else
  {
    maxbad = 0;
  }

  if (N > maxbad)
  {
    return sqrt(Q/N);
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "no data found in sdevXy";
    return _missing;
  }

}

//----------------------------------------------------------------
double GridAlgs::localCenteredSdev(int x, int y,
				   int sx, int sy, bool needHalf) const
{
  return localBoxSdev(x-sx, y-sy,  sx*2+1, sy*2 + 1, needHalf);
}

//----------------------------------------------------------------
void GridAlgs::sdev(int xw, int yw)
{

  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);

  // make a copy of the local object
  GridAlgs tmp(*this);

  Grid2dLoopAlgSdev A;

  // // do this computation writing to local object
  while (G.increment(tmp, A))
  {
    int x, y;
    double result;
    if (G.getXyAndResult(A, xw*yw/2, x, y, result))
    {
      _data[y*_nx + x] = result;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::sdevSimple(int xw, int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  // for each point, set value to standard deviation in a xw by yw window around
  // the point
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp.localCenteredSdev(x, y, xw, yw);
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::sdevThreaded(int sx, int sy, int numThread)
{
  GridAlgThreads *thread = new GridAlgs::GridAlgThreads();
  thread->init(numThread, false);

  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::SDEV,
					  sx, sy, iy, this, tmp);
    thread->thread(iy, (void *)info);
  }
  thread->waitForThreads();
  delete thread;
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::sdevNoOverlap(int xw, int yw)
{
  // make a copy of the local object
  GridAlgs tmp(*this);

  for (int y=0; y<_ny; y += yw)
  {
    for (int x=0; x<_nx; x += xw)
    {
      double sdev = tmp.localBoxSdev(x, y, xw, yw);
      _fillBox(x, y, xw, yw, sdev);
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::reduce(int f)
{
  if (f < 2)
  {
    return;
  }
  int nx = _nx/f;
  int ny = _ny/f;
  GridAlgs g(_name.c_str(), nx, ny, _missing);

  for (int y=0; y<ny; ++y)
  {
    int full_y = y*f;
    if (full_y >= _ny)
    {
      printf("ERROR\n");
      continue;
    }
    for (int x=0; x<nx; ++x)
    {
      int full_x = x*f;
      if (full_x >= _nx)
      {
	printf("ERROR\n");
	continue;
      }
      g.setValue(x, y, getValue(full_x, full_y));
    }
  }
  *this = g;
}

//---------------------------------------------------------------------------
void GridAlgs::reduceMax(int f)
{
  if (f < 2)
  {
    return;
  }
  int nx = _nx/f;
  int ny = _ny/f;
  GridAlgs g(_name.c_str(), nx, ny, _missing);
  for (int y=0; y<ny; ++y)
  {
    int full_y = y*f;
    if (full_y >= _ny)
    {
      printf("ERROR\n");
      continue;
    }
    for (int x=0; x<nx; ++x)
    {
      int full_x = x*f;
      if (full_x >= _nx)
      {
	printf("ERROR\n");
	continue;
      }
      g.setValue(x, y, localMax(full_x, full_y, f, f));
    }
  }
  *this = g;
  }

//---------------------------------------------------------------------------
void GridAlgs::reduceMax(int fx, int fy)
{
  if (fx < 2 && fy < 2)
  {
    return;
  }
  int nx = _nx/fx;
  int ny = _ny/fy;
  GridAlgs g(_name.c_str(), nx, ny, _missing);

  for (int y=0; y<ny; ++y)
  {
    int full_y = y*fy;
    if (full_y >= _ny)
    {
      printf("ERROR\n");
      continue;
    }
    for (int x=0; x<nx; ++x)
    {
      int full_x = x*fx;
      if (full_x >= _nx)
      {
	printf("ERROR\n");
	continue;
      }
      g.setValue(x, y, localMax(full_x, full_y, fx, fy));
    }
  }
  *this = g;
}

//---------------------------------------------------------------------------
bool GridAlgs::interpolate(const Grid2d &lowres, int res)
{
  if (lowres._nx != _nx/res)
  {
    printf("ERROR interpolating, bad dimensions\n");
    return false;
  }
  if (lowres._ny != _ny/res)
  {
    printf("ERROR interpolating, bad dimensions\n");
    return false;
  }
  for (int y=0; y<_ny; ++y)
  {
    int ry0 = y/res;
    for (int x=0; x<_nx; ++x)
    {
      int rx0 = x/res;
      double v = _bilinear(ry0, rx0, res, y, x, lowres);
      setValue(x, y, v);
    }
  }
  return true;
}

//----------------------------------------------------------------
void GridAlgs::dilate(int xw, int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  // for each point, set value to max in a xw by yw window around
  // the point
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp._max(x, y, xw, yw);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::dilateOneValue(double v, int xw, int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp._maxOneValue(v, x, y, xw, yw);
    }
  }
}


//----------------------------------------------------------------
double GridAlgs::localMedian(int xLwr, int xUpr, int yLwr, int yUpr)
{

  double v;
  vector <double> aList;

  for (int iy=yLwr; iy<=yUpr; iy++)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix=xLwr; ix<=xUpr; ix++)
    {
      if (ix < 0 || ix >= _nx)
      {
        continue;
      }
      if (getValue(ix, iy, v))
      {
        aList.push_back(v);
      }
    }
  }

  if (aList.empty())
  {
    return _missing;
  }

  int ind = aList.size() / 2;
  nth_element(aList.begin(), aList.begin()+ind, aList.end());
  return aList[ind];
}

//----------------------------------------------------------------
void GridAlgs::medianSimple(int xw, int yw, double bin_min, double bin_max,
			    double bin_delta)
{
  // make a fast median object
  Grid2dMedian F(*this, xw, yw, bin_delta, bin_min, bin_max);

  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // do this computation writing to local object
  // get the value
  int x, y;
  double m;

  // set value into 
  m = _median2(*this, xw, yw, F, G);
  G.getXy(x, y);
  setValue(x, y, m);

  while (G.increment())
  {
    m = _median2(*this, xw, yw, F, G);
    G.getXy(x, y);
    setValue(x, y, m);
  }
}

//----------------------------------------------------------------
void GridAlgs::median(int xw, int yw, double bin_min, double bin_max,
		      double bin_delta)
{
  // make a fast median object
  // Grid2dMedian F(*this, xw, yw, bin_delta, bin_min, bin_max);

  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);
  GridAlgs tmp(*this);
  Grid2dLoopAlgMedian A(bin_min, bin_max, bin_delta);

  while (G.increment(tmp, A))
  {
    int x, y;
    double m;
    if (G.getXyAndResult(A, xw*yw/2, x,  y, m))
    {
      _data[y*_nx + x] = m;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::medianNoOverlap(int xw, int yw, double bin_min, double bin_max,
			       double bin_delta, bool allow_any_data)
{
  // make a fast median object
  Grid2dMedian F(*this, xw, yw, bin_delta, bin_min, bin_max);

  // make a copy of the local object
  GridAlgs tmp(*this);

  for (int y=0; y<_ny; y += yw)
  {
    for (int x=0; x<_nx; x += xw)
    {
      double v = _medianInBox(tmp, x, y, xw, yw, allow_any_data, F);
      _fillBox(x, y, xw, yw, v);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::medianEntireDomain(double binMin, double binMax, double binDelta,
				  bool mask)
{
  // make a copy of the local object
  GridAlgs tmp(*this);

  std::vector<double> bin;
  std::vector<double> counts;
  int nc = 0;
  int nbin = static_cast<int>((binMax-binMin)/binDelta) + 1;
  for (int i=0; i<nbin; ++i)
  {
    double v = binMin + binDelta*i;
    bin.push_back(v);
    counts.push_back(0.0);
  }
  
  for (int y=0; y<_ny; y ++)
  {
    for (int x=0; x<_nx; x ++)
    {
      double v;
      if (getValue(x, y, v))
      {
	int index = static_cast<int>((v-binMin)/binDelta);
	if (index < 0)
	{
	  index = 0;
	}
	if (index >= nbin)
	{
	  index = nbin-1;
	}
	counts[index] ++;
	++nc;
      }
    }
  }

  if (nc == 0)
  {
    setAllMissing();
  }
  else
  {
    bool isSet = false;
    double median;
    double fpt = 0.50*static_cast<double>(nc);
    int ipt = static_cast<int>(fpt);
    int count = 0;
    for (int i=0; i<nbin; ++i)
    {
      count += static_cast<int>(counts[i]);
      if (count >= ipt)
      {
	isSet = true;
	median = bin[i];
	break;
      }
    }
    if (!isSet)
    {
      setAllMissing();
    }
    else
    {
      setAllToValue(median);
      if (mask)
      {
	maskMissingToMissing(tmp);
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::speckle(int xw, int yw, double bin_min, double bin_max,
		       double bin_delta)
{
  // make a fast median object
  // Grid2dMedian F(*this, xw, yw, bin_delta, bin_min, bin_max);

  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);
  GridAlgs tmp(*this);
  Grid2dLoopAlgSpeckle A(bin_min, bin_max, bin_delta);

  while (G.increment(tmp, A))
  {
    int x, y;
    double m;
    if (G.getXyAndResult(A, xw*yw/2, x,  y, m))
    {
      _data[y*_nx + x] = m;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::speckleInterest(int xw, int yw, double bin_min, double bin_max,
			       double bin_delta, const FuzzyF &fuzzyDataDiff,
			       const FuzzyF &fuzzyCountPctDiff)
{
  // make a fast median object
  // Grid2dMedian F(*this, xw, yw, bin_delta, bin_min, bin_max);

  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);
  GridAlgs tmp(*this);
  Grid2dLoopAlgSpeckleInterest A(bin_min, bin_max, bin_delta,
				 fuzzyDataDiff, fuzzyCountPctDiff);

  while (G.increment(tmp, A))
  {
    int x, y;
    double m;
    if (G.getXyAndResult(A, xw*yw/2, x,  y, m))
    {
      _data[y*_nx + x] = m;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//----------------------------------------------------------------
double GridAlgs::localCenteredTexture(int x, int y, int xw, int yw, bool isX,
				      bool needHalf) const
{
  double A=0.0;
  int n = 0;

  int maxbad;
  if (needHalf)
  {
    // slightly relaxed
    maxbad = (xw-1)*(yw-1)/2;
  }
  else
  {
    maxbad = 0;
  }

  for (int iy=y-yw; iy<=y+yw; ++iy)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    if (isX && (iy-1 < 0 || iy-1 >= _ny))
    {
      continue;
    }
    for (int ix=x-xw; ix<=x+xw; ++ix)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      if ((!isX) && (ix-1 < 0 || ix-1 >= _nx))
      {
	continue;
      }

      double v1,v2;
      bool ok;
      if (isX)
      {
	ok = getValue(ix, iy, v1) && getValue(ix,iy-1,v2);
      }
      else
      {
	ok = getValue(ix, iy, v1) && getValue(ix-1,iy,v2);
      }
      if (ok)
      {
	A = A+(v1-v2)*(v1-v2);
	n++;
      }
    }
  }
  if (n > maxbad && n != 0)
  {
    return A/static_cast<double>(n);//yw*xw-1);
  }
  else
  {
    return _missing;
  }
}

//---------------------------------------------------------------------------
void GridAlgs::textureThreaded(int sx, int sy, int numThread, bool isX)
{
  GridAlgThreads *thread = new GridAlgs::GridAlgThreads();
  thread->init(numThread, false);

  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    if (isX)
    {
      GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::TEXTURE_X,
					    sx, sy, iy, this, tmp);
      thread->thread(iy, (void *)info);
    }
    else
    {
      GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::TEXTURE_Y,
					    sx, sy, iy, this, tmp);
      thread->thread(iy, (void *)info);
    }
  }
  thread->waitForThreads();
  delete thread;
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::texture(int xw, int yw, bool isX)
{
  // make an object to loop through the grid
  Grid2dLoopA G(_nx, _ny, xw, yw);

  // make a copy of the local object
  GridAlgs tmp(*this);

  Grid2dLoopAlgTexture A(isX);


  // // do this computation writing to local object
  while (G.increment(tmp, A))
  {
    int x, y;
    double result;
    if (G.getXyAndResult(A, xw*yw/2, x, y, result))
    {
      _data[y*_nx + x] = result;
    }
    else
    {
      _data[y*_nx + x] = _missing;
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::db2linear(void)
{
  double v1,v2;

  // for each point, convert from db to linear
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      if( getValue(x, y, v1) )
      {
        v2 = v1/10;
        _data[y*_nx + x] = pow(10,v2); 
      }
      else
      {
	_data[y*_nx + x] = _missing; 
      }
    }
  }

}

//----------------------------------------------------------------
void GridAlgs::linear2db(void)
{

  double v1;

  // for each point, convert from linear to db
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      if (getValue(x,y,v1))
      {
	if (v1 != 0)
	{
	  _data[y*_nx + x] = 10*log10(v1);
	}
	else
	{
	  _data[y*_nx + x] = _missing; 
	}
      }
      else
      {
	_data[y*_nx + x] = _missing; 
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::xMaxForAllY(int nx)
{
  GridAlgs tmp(*this);
  for (int x=0; x<_nx; ++x)
  {
    double max=_missing, v;
    bool first = true;
    for (int ix=x-nx; ix<=x+nx; ++ix)
    {
      if (ix < 0 || ix >= _nx-1)
      {
	continue;
      }
      for (int iy=0; iy<_ny; ++iy)
      {
	if (getValue(ix, iy, v))
	{
	  if (first)
	  {
	    max = v;
	    first = false;
	  }
	  else
	  {
	    if (v > max)
	    {
	      max = v;
	    }
	  }
	}
      }
    }
    for (int iy=0; iy<_ny; ++iy)
    {
      tmp.setValue(x, iy, max);
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::xAverageForAllY(int nx)
{
  GridAlgs tmp(*this);
  for (int x=0; x<_nx; ++x)
  {
    double ave=0.0, count=0.0, v;
    for (int ix=x-nx; ix<=x+nx; ++ix)
    {
      if (ix < 0 || ix >= _nx-1)
      {
	continue;
      }
      for (int iy=0; iy<_ny; ++iy)
      {
	if (getValue(ix, iy, v))
	{
	  ave += v;
	  count ++;
	}
      }
    }
    if (count > 0)
    {
      ave = ave/count;
    }
    else
    {
      ave = _missing;
    }
    for (int iy=0; iy<_ny; ++iy)
    {
      tmp.setValue(x, iy, ave);
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::xMaxForAllY(int nx, double min_v)
{
  GridAlgs tmp(*this);
  for (int x=0; x<_nx; ++x)
  {
    double max=_missing, v;
    bool first = true;

    for (int iy=0; iy<_ny; ++iy)
    {
      if (getValue(x, iy, v))
      {
	if (v >= min_v)
	{
	  if (first)
	  {
	    max = v;
	    first = false;
	  }
	  else
	  {
	    if (v > max)
	    {
	      max = v;
	    }
	  }
	}
      }
    }
    if (!first)
    {      
      for (int ix=x-nx; ix<=x+nx; ++ix)
      {
	if (ix < 0 || ix >= _nx-1 || ix == x)
	{
	  continue;
	}
	for (int iy=0; iy<_ny; ++iy)
	{
	  if (getValue(ix, iy, v))
	  {
	    if (v >= min_v)
	    {
		if (v > max)
		{
		  max = v;
		}
	    }
	  }
	}
      }
    }
    for (int iy=0; iy<_ny; ++iy)
    {
      tmp.setValue(x, iy, max);
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::xAverageForAllY(int nx, double min_v)
{
  GridAlgs tmp(*this);
  for (int x=0; x<_nx; ++x)
  {
    double ave=0.0, count=0.0, v;
    for (int iy=0; iy<_ny; ++iy)
    {
      if (getValue(x, iy, v))
      {
	if (v >= min_v)
	{
	  ave += v;
	  count ++;
	}
      }
    }
    if (count > 0)
    {      
      for (int ix=x-nx; ix<=x+nx; ++ix)
      {
	if (ix < 0 || ix >= _nx-1 || ix == x)
	{
	  continue;
	}
	for (int iy=0; iy<_ny; ++iy)
	{
	  if (getValue(ix, iy, v))
	  {
	    if (v >= min_v)
	    {
	      ave += v;
	      count ++;
	    }
	  }
	}
      }
    }
    if (count > 0)
    {
      ave = ave/count;
    }
    else
    {
      ave = _missing;
    }
    for (int iy=0; iy<_ny; ++iy)
    {
      tmp.setValue(x, iy, ave);
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::xPcntGeForAllY(int nx, double min_v, double min_pct, int &x0,
			      int &x1)
{
  GridAlgs tmp(*this);
  x0 = x1 = -1;
  for (int x=0; x<_nx; ++x)
  {
    double npt=0.0, count=0.0, v;
    for (int ix=x-nx; ix<=x+nx; ++ix)
    {
      if (ix < 0 || ix >= _nx-1)
      {
	continue;
      }
      for (int iy=0; iy<_ny; ++iy)
      {
	count ++;
	if (getValue(ix, iy, v))
	{
	  if (v >= min_v)
	  {
	    npt ++;
	  }
	}
      }
    }
    npt = npt/count;
    for (int iy=0; iy<_ny; ++iy)
    {
      tmp.setValue(x, iy, npt);
    }
    if (npt >= min_pct)
    {
      if (x0 == -1)
      {
	x0 = x1 = x;
      }
      else
      {
	x1 = x;
      }
    }
  }
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::clumpFilter(void)
{
  // find points that have only 1 neighbor and remove
  bool did_change = true;
  while (did_change)
  {
    did_change = false;
    for (int iy=0; iy<_ny; ++iy)
    {
      for (int ix=0; ix<_nx; ++ix)
      {
	if (isValidWithAtMostOneNeighbor(ix, iy))
	{
	  setMissing(ix, iy);
	  did_change = true;
	}
	for (int nh=1; nh<=3; ++nh)
	{
	  if (_fillHole(nh, ix, iy))
	  {
	    did_change = true;
	  }
	}
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::clearNear(int x, int y, int n)
{
  for (int iy=y-n; iy<=y+n; ++iy)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix=x-n; ix<=x+n; ++ix)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      setValue(ix, iy, _missing);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::shiftX(int npt)
{
  double v;
  if (npt > 0)
  {
    for (int x=_nx -1 - npt; x>=0; --x)
    {
      for (int y=0; y<_ny; ++y)
      {
	v = getValue(x, y);
	setValue(x+npt, y, v);
      }
    }
    for (int x=0; x<npt; ++x)
    {
      for (int y=0; y<_ny; ++y)
      {
	setValue(x, y, _missing);
      }
    }
  }
  else if (npt < 0)
  {
    int m = -npt;
    for (int x=m; x<_nx; ++x)
    {
      for (int y=0; y<_ny; ++y)
      {
	v = getValue(x, y);
	setValue(x-m, y, v);
      }
    }
    for (int x=_nx-1-m; x<_nx; ++x)
    {
      for (int y=0; y<_ny; ++y)
      {
	setValue(x, y, _missing);
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::shiftY(int n)
{
  vector<double> newData = _data;
  for (int y=0; y<_ny; ++y)
  {
    int iy = y - n;
    while (iy < 0)
    {
      iy += _ny;
    }
    while (iy >= _ny)
    {
      iy -= _ny;
    }
    for (int ix=0; ix<_nx; ++ix)
    {
      newData[_ipt(ix, iy)] = _data[_ipt(ix, y)];
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::boxExpand(void)
{
  GridAlgs tmp(*this);

  double v;

  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      if (getValue(x, y, v))
      {
	tmp.setValue(x, y, v);
	for (int yi=y-1; yi<=y+1; ++yi)
	{
	  if (yi < 0 || yi >= _ny)
	  {
	    continue;
	  }
	  for (int xi=x-1; xi<=x+1; ++xi)
	  {
	    if (xi < 0 || xi >= _nx)
	    {
	      continue;
	    }
	    tmp.setValue(xi,yi,v);
	  }
	}
      }
    }
  }
  *this = tmp;
}	


/*----------------------------------------------------------------*/
void GridAlgs::roundToNearest(double res, double r0, double r1)
{
  for (int i=0; i<_npt; ++i)
  {
    double v;
    if (!getValue(i, v))
      continue;
    double vnew = _round(v, r0, r1, res);
    setValue(i, vnew);
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::thinlineMask(const Grid2d &data, double min_data, 
			   double min_area, double mask_value)
{
  setAllMissing();

  Grid2dOffset o(sqrt(min_area)/2.0, 0.0, _nx, data._missing);
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      if (o.percentIsBad(0.4, data, x, y))
      {
	continue;
      }
      double v;
      if (o.average(data, x, y, v))
      {
	if (v < min_data)
	  continue;
      }
      setValue(x, y, mask_value);
    }
  }
}

/*----------------------------------------------------------------*/
void GridAlgs::copyMissingToInput(Grid2d &data) const
{
  for (int i=0; i<_npt; ++i)
  {
    if (isMissing(i))
    {
      data.setMissing(i);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::belowThresholdToMissing(double thresh)
{
  for (int i=0; i<_npt; ++i)
  {
    double a;
    if (getValue(i, a))
    {
      if (a < thresh)
      {
	setMissing(i);
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::thresholdMask(const Grid2d &thresh)
{
  for (int i=0; i<_npt; ++i)
  {
    double a;
    if (getValue(i, a))
    {
      double t;
      if (thresh.getValue(i, t))
      {
	if (a < t)
	{
	  setMissing(i);
	}
      }
      else
      {
	// assume missing threshold is same as a low threshold,
	// do nothing
      }
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::invert(double value)
{
  if (value == _missing)
  {
    value = value+1;
  }
  for (int i=0; i<_npt; ++i)
  {
    if (isMissing(i))
    {
      setValue(i, value);
    }
    else
    {
      setMissing(i);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::maskExcept(double v)
{
  for (int i=0; i<_npt; ++i)
  {
    double vi;
    if (getValue(i, vi))
    {
      if (vi != v)
      {
	setMissing(i);
      }
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::fuzzyRemap(const FuzzyF &fuzzy)
{
  for (int i=0; i<_npt; ++i)
  {
    double vi;
    if (getValue(i, vi))
    {
      vi = fuzzy.apply(vi);
      setValue(i, vi);
    }
  }
}

//-----------------------------------------------------------------
void GridAlgs::FIRfilter(const std::vector<double> &coeff)
{
  for (int y=0; y<_ny; ++y)
  {
    _FIRfilterY(y, coeff);
  }
}

//-----------------------------------------------------------------
void GridAlgs::nptBetweenGoodDataPointsX(const Grid2d &clumps,
					 const Grid2d &data,
					 int minPt)
{
  // set this missing everywhere
  setAllMissing();
  
  for (int y=0; y<_ny; ++y)
  {
    _nptBetweenGoodDataPointsAlongY(y, clumps, data, minPt);
  }
}

//-----------------------------------------------------------------
void GridAlgs::totalAttenuation(const Grid2d &clumps, const Grid2d &dwr,
				double minKm, double kmPerX)
{
  // set this missing everywhere
  setAllMissing();
  for (int y=0; y<_ny; ++y)
  {
    _totalAttenuationAlongY(y, clumps, dwr, minKm, kmPerX);
  }
}

//-----------------------------------------------------------------
void GridAlgs::averageAttenuation(const Grid2d &extent, const Grid2d &atotal)
{
  GridAlgs counts(extent);

  setAllMissing();
  counts.setAllToValue(0.0);
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      double v, e;
      if (atotal.getValue(x, y, v) && extent.getValue(x, y, e))
      {
	for (int xi=x; xi<=x+(int)e; ++xi)
	{
	  if (isMissing(xi, y))
	  {
	    setValue(xi, y, v);
	  }
	  else
	  {
	    increment(xi, y, v);
	  }
	  counts.increment(xi, y, 1.0);
	}
      }
    }
    // 2nd pass to normalize
    for (int x=0; x<_nx; ++x)
    {
      double v, c;
      if (getValue(x, y, v) && counts.getValue(x, y, c))
      {
	if (c == 0.0)
	{
	  LOG(WARNING) << "value but no count";
	}
	else
	{
	  setValue(x, y, v/c);
	}
      }
    }
  }
}


//-----------------------------------------------------------------
void GridAlgs::sumZ(const Grid2d &Z, const Grid2d &extent, double p)
{
  setAllMissing();
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      double v;
      if (extent.getValue(x, y, v))
      {
	// sum up Z along this x out to extent
	int e = (int)v;
	double sum = 0.0;
	for (int xi = x; xi<=x+e; ++xi)
	{
	  if (Z.getValue(xi,y,v))
	  {
	    sum += pow(v, p);
	  }
	}
	setValue(x, y, sum);
      }
    }
  }
}  

//-----------------------------------------------------------------
void GridAlgs::weightedAverage(const std::vector<Grid2d> &inputs,
			       const std::vector<double> &weights,
			       bool normalize)
{
  *this = inputs[0];
  setAllMissing();

  // create a sum of weights grid
  GridAlgs sumWeights(inputs[0]);
  sumWeights.setAllToValue(0.0);

  for (size_t i=0; i<inputs.size(); ++i)
  {
    double w = weights[i];

    // multiply this grid by the weight
    GridAlgs gi(inputs[i]);
    gi.multiply(w);

    // add w to sumWeights at all points where inputs[i] not missing
    sumWeights.incrementInMask(inputs[i], w);
    
    // now add gi to the overall final grid
    add(gi);
  }

  if (normalize)
  {
    divide(sumWeights);
  }
}

//-----------------------------------------------------------------
void GridAlgs::weightedAngleAverage(const std::vector<Grid2d> &inputs,
				   const std::vector<double> &weights,
				   bool is360)
{
  AngleCombiner angles(weights, is360);
  
  *this = inputs[0];
  setAllMissing();

  for (int ipt=0; ipt<inputs[0].getNdata(); ++ipt)
  {
    angles.clearValues();
    for (int i=0; i<(int)inputs.size(); ++i)
    {
      double v;
      if (inputs[i].getValue(ipt, v))
      {
	angles.setGood(i, v, 1.0);
      }
      else
      {
	angles.setBad(i);
      }
    }
    double a;
    if (angles.getCombineAngle(a))
    {
      setValue(ipt, a);
    }
  }
}

//-----------------------------------------------------------------
void GridAlgs::maxExpand(int nx, int ny)
{
  dilate(nx, ny);
}

//-----------------------------------------------------------------
void GridAlgs::expandLaterally(double npt)
{
  Grid2d tmp(*this);

  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_ny; ++x)
    {
      double a;
      if (getValue(x, y, a))
      {
	Line line(a, 2*npt+1);
	// move the line to x,y
	line.move(x, y);
        PointList points = line.xyValues();
	points.toGrid(tmp, a);
      }
    }
  }
  dataCopy(tmp);
}

//-----------------------------------------------------------------
void GridAlgs::_nptBetweenGoodDataPointsAlongY(int y, const Grid2d &clumps,
					       const Grid2d &data,
					       int minPt)
{
  // find all the contiguous clumps along this y
  bool outside = true;
  int x0, x1;
  double value;
  vector< pair<int,int> >  runs;
  for (int x=0; x<_nx; ++x)
  {
    double v;
    if (clumps.getValue(x,y,v))
    {
      if (outside)
      {
	outside = false;
	x0 = x1 = x;
	value = v;
      }
      else
      {
	if (v == value)
	{
	  x1 = x;
	}
	else
	{
	  // end of that run, store
	  pair<int,int> x0x1(x0,x1);
	  runs.push_back(x0x1);

	  // start new run
	  x0 = x1 = x;
	  value = v;
	}
      }
    }
    else
    {
      if (!outside)
      {
	outside = true;
	pair<int,int> x0x1(x0,x1);
	runs.push_back(x0x1);
      }
    }
  }

  // for each run:
  for (size_t i=0; i<runs.size(); ++i)
  {
    _nptBetweenGoodDataPointsAlongYSubset(runs[i].first, runs[i].second, y,
					  data, minPt);
  }    
}

//-----------------------------------------------------------------
void GridAlgs::_totalAttenuationAlongY(int y, const Grid2d &clumps,
				       const Grid2d &dwr, double minKm,
				       double kmPerX)
{
  // find all the contiguous clumps along this y
  bool outside = true;
  int x0, x1;
  double value;
  vector< pair<int,int> >  runs;
  for (int x=0; x<_nx; ++x)
  {
    double v;
    if (clumps.getValue(x,y,v))
    {
      if (outside)
      {
	outside = false;
	x0 = x1 = x;
	value = v;
      }
      else
      {
	if (v == value)
	{
	  x1 = x;
	}
	else
	{
	  // end of that run, store
	  pair<int,int> x0x1(x0,x1);
	  runs.push_back(x0x1);

	  // start new run
	  x0 = x1 = x;
	  value = v;
	}
      }
    }
    else
    {
      if (!outside)
      {
	outside = true;
	pair<int,int> x0x1(x0,x1);
	runs.push_back(x0x1);
      }
    }
  }

  // for each run:
  for (size_t i=0; i<runs.size(); ++i)
  {
    _totalAttenuationAlongYSubset(runs[i].first, runs[i].second, y,
				  dwr, minKm, kmPerX);
  }    
}

//-----------------------------------------------------------------
void GridAlgs::_nptBetweenGoodDataPointsAlongYSubset(int x0, int x1, int y,
						     const Grid2d &data,
						     int minPt)
{
  for (int x=x0; x<=x1; ++x)
  {
    double v;
    if (!data.getValue(x, y, v))
    {
      continue;
    }

    for (int xi=x+1; xi<=x1; ++xi)
    {
      double vi;
      if (data.getValue(xi, y, vi))
      {
	int extent = (xi - x);
	if (extent >= minPt)
	{
	  // ok, good at this point so can set value into local Grid2d
	  setValue(x, y, (double)extent);
	  break;
	}
      }
    }
    // break to here after setting value, or never setting a value,
    // ready to increment by one and try again
  }
}

//-----------------------------------------------------------------
void GridAlgs::_totalAttenuationAlongYSubset(int x0, int x1, int y,
					     const Grid2d &dwr, double minKm,
					     double kmPerX)
{
  double vStart;
  for (int x=x0; x<=x1; ++x)
  {
    double v;
    if (!dwr.getValue(x, y, v))
    {
      continue;
    }

    // go ahead and compute an attenuation if you can and store to x,y
    vStart = v;
    for (int xi=x+1; xi<=x1; ++xi)
    {
      double vi;
      if (dwr.getValue(xi, y, vi))
      {
	double extent = (xi - x)*kmPerX;
	if (extent >= minKm)
	{
	  // ok, good at this point so can set value into local Grid2d
	  setValue(x, y, (vi - vStart));
	  break;
	}
      }
    }
    // break to here after setting value, or never set the attenuation at x, y
    // ready to increment by one and try again
  }
}

//-----------------------------------------------------------------
void GridAlgs::_FIRfilterY(int y, const std::vector<double> &coeff)
{
  // find index of first and last valid data
  int i0 = _firstValidIndex(y);
  int i1 = _lastValidIndex(y);
  if (i0 < 0 || i1 < 0)
  {
    LOG(WARNING) << "FIRfilter All the data is missing, no filtering y=" << y;
    return;
  }

  int nCoeff = static_cast<int>(coeff.size());
  if (i1-i0+1 < nCoeff*2)
  {
    LOG(WARNING) << "FIRfilter data mostly missing only " << i1-i0+1
	       << " good values";
    return;
  }
  LOG(DEBUG_VERBOSE) << "FIRfilter  I0,I1=" << i0 << "," << i1;

  // get the center index value
  int centerCoeff = nCoeff/2;
  if (nCoeff % 2)
  {
    // odd # of coeffs...good
  }
  else
  {
    LOG(WARNING) << "FIRfilter even number of coeff, use n/2'th as center";
  }

  // if interpolating at edges, do a linear regression to get coefficients
  double m0=0, int0=0, m1=0, int1=0;
  bool allbad0=true, allbad1=true;
  allbad0 = !_linearRegression(y, i0, i1, 20, true, m0, int0);
  allbad1 = !_linearRegression(y, i0, i1, 20, false, m1, int1);

  // create a vector that extends at each end
  vector<double> tmpData = _extendData(y, i0, i1, centerCoeff, nCoeff,
				       allbad0, m0, int0, allbad1, m1, int1);

  // do gap filling on this data
  vector<double> gapFilledData = tmpData;
  _fillGaps(gapFilledData, _missing);

  // create a vector to store data with gaps filled, compute sum of coefficients
  double sumCoeff = 0.0;
  for (int i=0; i<nCoeff; ++i)
  {
    sumCoeff += coeff[i];
  }

  for (int j=0; j<_nx; ++j)
  {
    _applyFIR(j, y, i0, i1, centerCoeff, tmpData,
	      gapFilledData, coeff, sumCoeff);
  }
}

//---------------------------------------------------------------------------
void GridAlgs::compute(void *ti)
{
  GridAlgsInfo *info = static_cast<GridAlgsInfo *>(ti);
  switch (info->_type)
  {
  case GridAlgsInfo::SMOOTH:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->localCenteredAverage(ix, info->_y,
						       info->_sx, info->_sy,
						       true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::SDEV:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->localCenteredSdev(ix, info->_y,
						    info->_sx, info->_sy,
						    true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::TEXTURE_X:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->localCenteredTexture(ix, info->_y,
						       info->_sx, info->_sy,
						       true, true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::TEXTURE_Y:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->localCenteredTexture(ix, info->_y, info->_sx,
						       info->_sy,
						       false, true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  default:
    break;
  }
  delete info;
}

//----------------------------------------------------------------
// return ordered (based on taxicab metric distance, small to large)
// the indices into the box at radius r from x,y.
std::vector<int> GridAlgs::_orderedIndices(int x, int y, int r) const
{
  std::vector<int> o;
  int x0, x1, y0, y1, i;
    
  if (r < 1)
  {
    return o;
  }
    
  // get box positions.
  x0 = x - r;
  x1 = x + r;
  y0 = y - r;
  y1 = y + r;
    
  // the nearest are straight out in all 4 directions.
  _appendIfOk(x, y0, o);
  _appendIfOk(x, y1, o);
  _appendIfOk(x0, y, o);
  _appendIfOk(x1, y, o);

  // next over 1, etc. till hit ends.
  for (i=1;i<=r;++i)
  {
    _appendIfOk(x-i, y0, o);
    _appendIfOk(x-i, y1, o);
    _appendIfOk(x+i, y0, o);
    _appendIfOk(x+i, y1, o);

    _appendIfOk(x0, y-i, o);
    _appendIfOk(x1, y-i, o);
    _appendIfOk(x0, y+i, o);
    _appendIfOk(x1, y+i, o);
  }
  return o;
}

/*----------------------------------------------------------------*/
void GridAlgs::_appendIfOk(int x, int y, std::vector<int> &o) const
{
  if (x < 0 || x >= _nx || y < 0 || y >= _ny)
  {
    return;
  }
  int offset = _ipt(x, y);
  o.push_back(offset);
}

/*----------------------------------------------------------------*/
double GridAlgs::_bilinear(int ry0, int rx0, int res, int y, int x, 
			   const Grid2d &lowres)
{
  int x0 = rx0*res;
  int y0 = ry0*res;
  int x1 = x0 + res;
  int y1 = y0 + res;
  double f00=0, f01=0, f10=0, f11=0;
  int rx1 = rx0 + 1;
  int ry1 = ry0 + 1;
  if (lowres.inRange(rx0, ry0))
  {
    if (!lowres.getValue(rx0, ry0, f00))
    {
      f00 = 0;
    }
  }
  if (lowres.inRange(rx0, ry1))
  {
    if (!lowres.getValue(rx0, ry1, f01))
    {
      f01 = 0;
    }
  }
  if (lowres.inRange(rx1, ry0))
  {
    if (!lowres.getValue(rx1, ry0, f10))
    {
      f10 = 0;
    }
  }
  if (lowres.inRange(rx1, ry1))
  {
    if (!lowres.getValue(rx1, ry1, f11))
    {
      f11 = 0;
    }
  }

  double ret = (f00*(x1-x)*(y1-y) +
		f10*(x-x0)*(y1-y) +
		f01*(x1-x)*(y-y0) +
		f11*(x-x0)*(y-y0));
  ret = ret/((x1-x0)*(y1-y0));
  return ret;
}

//----------------------------------------------------------------
double GridAlgs::_max(int x, int y, int xw, int yw) const
{
  double max = 0.0;
  bool first = true;
  for (int iy=y-yw; iy<=y+yw; ++iy)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix=x-xw; ix<=x+xw; ++ix)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      double v;
      if (getValue(ix, iy, v))
      {
	if (first)
	{
	  first = false;
	  max = v;
	}
	else
	{
	  if (v > max) 
	  {
	    max = v;
	  }
	}
      }
    }
  }
  if (first)
  {
    return _missing;
  }
  else
  {
    return max;
  }
}

//----------------------------------------------------------------
double GridAlgs::_maxOneValue(double value, int x, int y, int xw, int yw) const
{
  double  v, v0;
  if (getValue(x, y, v0))
  {
    if (v0 == value)
    {
      return v0;
    }
  }
  else
  {
    v0 = _missing;
  }

  for (int iy=y-yw; iy<=y+yw; ++iy)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix=x-xw; ix<=x+xw; ++ix)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      if (ix == x && iy == y)
      {
	continue;
      }
      if (getValue(ix, iy, v))
      {
	if (v == value)
	{
	  return value;
	}
      }
    }
  }
  return v0;
}

//----------------------------------------------------------------
void GridAlgs::_fillEdge(int xw, int yw, double value)
{
  int xedge, yedge, x, y;

  xedge = xw >> 1;
  yedge = yw >> 1;
  for (y=0; y<_ny; y++)
  {
    if (y< _ny-yedge && y > yedge)
    {
      for (x = 0; x < xedge; x++)
      {
	setValue(x, y, value);
      }
      for (x= _nx - xedge; x < _nx; x++)
      {
	setValue(x, y, value);
      }
    }
    else
    {
      for (x = 0; x < _nx; x++)
      {
	setValue(x, y, value);
      }
    }
  }
}

//----------------------------------------------------------------
// fill hole horizontal or vertial with min point at ix,iy
bool GridAlgs::_fillHole(int n, int ix, int iy)
{
  if (ix < 1 || ix >= _nx - 1 - n)
  {
    return false;
  }
  if (iy < 1 || iy >= _ny - 1 - n)
  {
    return false;
  }
  
  if (!isMissing(ix, iy))
  {
    return false;
  }

  // could there be a horizontal hole here?
  bool hhole = true, vhole = true;
  for (int i=1; i<n; ++i)
  {
    if (!isMissing(ix+i, iy))
    {
      hhole = false;
    }
    if (!isMissing(ix, iy+i))
    {
      vhole = false;
    }
  }
  if (hhole)
  {
    if (_fillHhole(n, ix, iy))
    {
      return true;
    }
  }
  if (vhole)
  {
    if (_fillVhole(n, ix, iy))
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------
// fill hole horizontal with min point at ix,iy
// x values are missing on entry
bool GridAlgs::_fillHhole(int n, int ix, int iy)
{
  if (isMissing(ix-1, iy))
  {
    return false;
  }
  if (isMissing(ix+n, iy))
  {
    return false;
  }
  double  v = _data[iy*_nx + ix-1];
  bool minus = true, plus=true;
  for (int i=ix-1; i<=ix+n; ++i)
  {
    if (isMissing(i, iy-1))
    {
      minus = false;
    }
    if (isMissing(i, iy+1))
    {
      plus = false;
    }
    if (!minus && !plus)
    {
      return false;
    }
  }
  // it was minus or plus so fill now
  for (int i=0; i<n; ++i)
  {
    setValue(ix+i, iy, v);
  }
  return true;
}

//----------------------------------------------------------------
// fill hole vertical with min point at ix,iy
// y values are missing on entry
bool GridAlgs::_fillVhole(int n, int ix, int iy)
{
  if (isMissing(ix, iy-1))
  {
    return false;
  }
  if (isMissing(ix, iy+n))
  {
    return false;
  }
  double v = _data[(iy-1)*_nx + ix];
  bool minus = true, plus=true;
  for (int i=iy-1; i<=iy+n; ++i)
  {
    if (isMissing(ix-1, i))
    {
      minus = false;
    }
    if (isMissing(ix+1, i))
    {
      plus = false;
    }
    if (!minus && !plus)
    {
      return false;
    }
  }
  // it was minus or plus so fill now
  for (int i=0; i<n; ++i)
  {
    setValue(ix, iy+i, v);
  }
  return true;
}

//----------------------------------------------------------------
void GridAlgs::_fillBox(const int x0, const int y0, const int nx,
			const int ny, const double v)
{
  for (int y=y0; y<y0+ny; ++y)
  {
    if (y>=0 && y < _ny)
    {
      for (int x=x0; x<x0+nx; ++x)
      {
	if (x >= 0 && x < _nx)
	{
	  _data[_ipt(x, y)] = v;
	}
      }
    }
  }
}

//-----------------------------------------------------------------
bool GridAlgs::_linearRegression(int y, int i0, int i1, int npt, bool up,
				 double &slope, double &intercept) const
{
  // build up nptLinearInterp at each end of the data, skipping missing
  // data
  double N = 0;
  double sumxy=0, sumx=0, sumy=0,sumx2=0;
  int dataOffset=0;

  int for0, for1, delta;
  if (up)
  {
    for0 = 0;
    for1 = i1-i0+1;
    delta = 1;
    dataOffset = i0;
  }
  else
  {
    for0 = 0;
    for1 = -(i1-i0+1);
    delta = -1;
    dataOffset = i1;
  }

  for (int x=for0; x!=for1; x += delta)
  {
    double v;
    if (getValue(x+dataOffset, y, v))
    // if (_data[i+dataOffset] != _missing)
    {
      N++;
      double d_x = static_cast<double>(x);
      sumx += d_x;
      sumy += v; //_data[i+dataOffset];
      sumxy += d_x*v; //_data[i+dataOffset];
      sumx2 += d_x*d_x;
      if (N >= npt)
      {
	break;
      }
    }
  }
  if (N == npt)
  {
    slope = (N*sumxy - sumx*sumy)/(N*sumx2 - sumx*sumx);
    intercept = (sumy - slope*sumx)/N;
    return true;
  }
  else
  {
    return false;
  }
}


//-----------------------------------------------------------------
std::vector<double> GridAlgs::_extendData(int y, int i0, int i1,
					  int centerCoeff, 
					  int nCoeff, bool allbad0,
					  double m0, double int0,
					  bool allbad1,
					  double m1, double int1) const
{
  // copy the data, but expand at each end 
  vector<double> tmpData;
  int nTmp = i1-i0+1 + centerCoeff*2;

  tmpData.reserve(nTmp);

  for (int j=0; j<nTmp; ++j)
  {
    int eIndex = -centerCoeff+j;  // extension index
    int dIndex = eIndex + i0;     // data index,  crosses over to >=0 at i0
    if (j < centerCoeff)
    {
      tmpData.push_back(_extend(y, eIndex, m0, int0, allbad0));
    }
    else if (dIndex <= i1)
    {
      tmpData.push_back(_data[_ipt(dIndex, y)]);
    }
    else
    {
      int eIndP = dIndex - i1;
      tmpData.push_back(_extend(y, eIndP, m1, int1, allbad1));
    }
  }
  return tmpData;
}
    
//-----------------------------------------------------------------
double GridAlgs::_extend(int y, int interpIndex, double m, double intercept,
			 bool allbad) const
{
   double d = _missing;
   if (!allbad)
   {
     d = m*static_cast<double>(interpIndex) + intercept;
   }
   return d;
 }



//-----------------------------------------------------------------
void GridAlgs::_applyFIR(int x, int y, int i0, int i1, int centerCoeff, 
			 const std::vector<double> &tmpData,
			 const std::vector<double> &gapFilledData,
			 const std::vector<double> &coeff, double sumCoeff)
{
  int j = _ipt(x,y);

  int tIndex = x+centerCoeff-i0;
  if (tIndex < 0 || tIndex >= (int)tmpData.size())
  {
    _data[j] = _missing;
    return;
  }
  
  if (x < i0 || x > i1)
  {
    _data[j] = _missing;
    return;
  }
  if (_data[j] == _missing)
  {
    return;
  }
  LOG(DEBUG_VERBOSE) << "Interpolating data centered at " << j;

  double quality  = _FIRquality(centerCoeff, tmpData, gapFilledData, tIndex);
  if (quality > 0)
  {
    _data[j] = _sumProduct(coeff, sumCoeff, gapFilledData, tIndex-centerCoeff);
  }
  else
  {
    _data[j] = _missing;
  }
}


//-----------------------------------------------------------------
double GridAlgs::_FIRquality(int centerCoeff, const vector<double> &tmpData,
			     const vector<double> &gapFilledData,
			     int tIndex) const
{
  int n = 2*centerCoeff + 1;  // the FIR filter window size

  vector<double> qmeasure;
  qmeasure.reserve(n);
  qmeasure[0] = 1.0;
  qmeasure[1] = 0.95;
  qmeasure[2] = 0.90;
  qmeasure[3] = 0.85;
  qmeasure[4] = 0.80;
  qmeasure[5] = 0.75;
  for (int i=6; i<n/2; ++i)
  {
    qmeasure[i] = 0.5;
  }
  for (int i=n/2; i<n; ++i)
  {
    qmeasure[i] = 0.0;
  }

  int nbad = 0;
  for (int i=-centerCoeff; i<=centerCoeff; ++i)
  {
    int ind = tIndex + i;
    if (tmpData[ind] == _missing)
    {
      ++nbad;
    }
    if (gapFilledData[ind] == _missing)
    {
      // don't allow any missing data in gap filled data
      return 0.0;
    }
  }
  return qmeasure[nbad];
}

//-----------------------------------------------------------------
double GridAlgs::_sumProduct(const std::vector<double> &coeff, double sumCoeff,
			     const std::vector<double> &data, int i0) const
{
  double sumprod = 0.0;
  for (size_t i=0; i<coeff.size(); ++i)
  {
    if (data[i+i0] == _missing)
    {
      return _missing;
    }
    else
    {
      sumprod += coeff[i]*data[i+i0];
    }
  }
  return sumprod/sumCoeff;
}
  
