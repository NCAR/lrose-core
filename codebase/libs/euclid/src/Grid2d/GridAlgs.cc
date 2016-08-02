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
#include <euclid/Grid2dOffset.hh>
#include <euclid/Grid2dLoop.hh>
#include <euclid/Grid2dMedian.hh>
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
static double _median2(Grid2d &out, const int xw, const int yw,
		       Grid2dMedian &F, Grid2dLoop &G)
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

//----------------------------------------------------------------
static double _speckle(Grid2d &out, const int xw, const int yw,
		       Grid2dMedian &F, Grid2dLoop &G) 
{
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // update Grid2dMedian object
  F.update(newv, oldv);

  double p25, p75;
  p25 = F.getPercentile(0.25);
  p75 = F.getPercentile(0.75);
  if (p25 == F.getMissing() || p75 == F.getMissing())
  {
    return out.getMissing();
  }
  else
  {
    return p75 - p25;
  }
}

//----------------------------------------------------------------
static double _speckleInterest(Grid2d &out, const int xw, const int yw,
			       const FuzzyF &fuzzyDataDiff,
			       const FuzzyF &fuzzyCountPctDiff,
			       Grid2dMedian &F, Grid2dLoop &G) 
{
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // update Grid2dMedian object
  F.update(newv, oldv);

  double p25, p50, p75;
  p25 = F.getPercentile(0.25);
  p50 = F.getPercentile(0.50);
  p75 = F.getPercentile(0.75);

  double c25, c50, c75;
  c25 = F.getCount(0.25);
  c50 = F.getCount(0.50);
  c75 = F.getCount(0.75);
  double missing = F.getMissing();

  if (p25 == missing || p50 == missing || p75 == missing ||
      c25 == missing || c50 == missing || c75 == missing)
  {
    return out.getMissing();
  }
  else
  {
    double interest = 1.0;
    interest *= fuzzyDataDiff.apply(p50-p25);
    interest *= fuzzyDataDiff.apply(p75-p50);
    
    double num = F.getNum();
    c25 /= num;
    c50 /= num;
    c75 /= num;

    interest *= fuzzyCountPctDiff.apply(fabs(c50-c25));
    interest *= fuzzyCountPctDiff.apply(fabs(c75-c50));
    
    return interest;
  }
}

//----------------------------------------------------------------
static double _medianInBox(Grid2d &data, const int x0, const int y0,
			   const int nx, const int ny, 
			   const bool allow_any_data, Grid2dMedian &F)
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
TaThread *GridAlgs::GridAlgThreads::clone(const int index)
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
bool GridAlgs::isValidWithAtMostOneNeighbor(const int ix, const int iy) const
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
bool GridAlgs::isHoleSingle(const int ix, const int iy, double &v) const
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

//---------------------------------------------------------------------------
std::string GridAlgs::getInfoForAlg(const bool debug, double &missing,
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
bool GridAlgs::intersects(const int i, const Grid2d &g, int &ngood0, int &n0,
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
double GridAlgs::localMax(const int ix, const int iy, const int sx, 
			  const int sy) const
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
double GridAlgs::localAverage(const int ix, const int iy, const int sx, 
			      const int sy, bool needHalf) const
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
    maxbad = static_cast<double>(sx*sy)/2.0;
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
double GridAlgs::localAverageNoMissing(const int ix, const int iy,
				       const int sx,  const int sy) const
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

//---------------------------------------------------------------------------
double GridAlgs::averageAtX(const int ix) const
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
double GridAlgs::maxAtX(const int ix) const
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
void GridAlgs::multiply(const double value)
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
void GridAlgs::add(const double value)
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
void GridAlgs::smooth(const int sx, const int sy)
{
  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      double v = localAverage(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::smoothThreaded(const int sx, const int sy, int numThread)
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

//---------------------------------------------------------------------------
void GridAlgs::compute(void *ti)
{
  GridAlgsInfo *info = static_cast<GridAlgsInfo *>(ti);
  switch (info->_type)
  {
  case GridAlgsInfo::SMOOTH:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->localAverage(ix, info->_y, info->_sx, 
					       info->_sy, true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::SDEV:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->_sdev(ix, info->_y, info->_sx, info->_sy,
					true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::TEXTURE_X:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->_textureX(ix, info->_y, info->_sx, info->_sy,
					    true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  case GridAlgsInfo::TEXTURE_Y:
    for (int ix=0; ix<info->_gridAlgs->_nx; ++ix)
    {
      double v = info->_gridAlgs->_textureY(ix, info->_y, info->_sx, info->_sy,
					    true);
      info->_out.setValue(ix, info->_y, v);
    }
    break;
  default:
    break;
  }
  delete info;
}

//---------------------------------------------------------------------------
void GridAlgs::smoothNoMissing(const int sx, const int sy)
{
  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    for (int ix=0; ix<_nx; ++ix)
    {
      double v = localAverageNoMissing(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::fillGaps(const int sx, const int sy)
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
      double v = localAverage(ix, iy, sx, sy);
      tmp.setValue(ix, iy, v);
    }
  }
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::reduce(const int f)
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
void GridAlgs::reduceMax(const int f)
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
void GridAlgs::reduceMax(const int fx, const int fy)
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

#ifdef NOTDEF
//---------------------------------------------------------------------------
void GridAlgs::maxRTheta(const int circle_r, const double dx, const double dy,
		   const double x0, const int maxy)
{
  GridAlgs g(*this);
  //
  // for each radius, get lookup offsets and use them
  //
  // at a given radius the max number of azimuths is therefore computed
  // using this:
  // 
  //   distance azimuthally for M gridpoints = r*M*dy*3.14/180
  //   M*r*dy*3.14/180 <= circle_r*dx  gives maximum M to check out at r
  //
  //   total distance = (approx) sqrt((r2-r1)^2 + (r*(theta change)
  //
  for (int x=0; x<_nx; ++x)
  {
    Grid2dLookup G;
    G.init(x, circle_r, dx, dy, x0, maxy);
    for (int y=0; y<_ny; ++y)
    {
      g.setValue(x, y, G.max(*this, x, y));
    }
  }
  *this = g;
}
#endif


//---------------------------------------------------------------------------
bool GridAlgs::interpolate(const Grid2d &lowres, const int res)
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
void GridAlgs::dilate2(const int xw, const int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  // for each point, set value to max in a xw by yw window around
  // the point
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp._dilate(x, y, xw, yw);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::dilateOneValue(const double v, const int xw, const int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp._dilateOneValue(v, x, y, xw, yw);
    }
  }
}

//----------------------------------------------------------------
double GridAlgs::medianXy(const int xLwr, const int xUpr, const int yLwr,
			  const int yUpr)
{

  double v;
  vector <double> aList;

  for (int iy=0; iy<_ny; iy++)
  {
    if (iy < yLwr || iy >= yUpr)
    {
      continue;
    }
    for (int ix=0; ix<_nx; ix++)
    {
      if (ix < xLwr || ix >= xUpr)
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
double GridAlgs::meanXy(const int xLwr, const int xUpr, const int yLwr,
			const int yUpr)
{

  double v,A=0,N=0;

  for (int iy=0; iy<_ny; iy++)
  {
    if (iy < yLwr || iy >= yUpr)
    {
      continue;
    }
    for (int ix=0; ix<_nx; ix++)
    {
      if (ix < xLwr || ix >= xUpr)
      {
	continue;
      }
      if (getValue(ix, iy, v))
      {
        A += v;
        N += 1;
      }
    }
  }

  if (N > 0) 
  {
    return A/N;
  }
  else
  {
    LOG(ERROR) << "no data found in mean_xy"; 
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::sdevXy(const int xLwr, const int xUpr, const int yLwr,
			const int yUpr)
{

  double v, A=0,Q=0,N=0;

  for (int iy=0; iy<_ny; iy++)
  {
    if (iy < yLwr || iy >= yUpr)
    {
      continue;
    }
    for (int ix=0; ix<_nx; ix++)
    {
      if (ix < xLwr || ix >= xUpr)
      {
	continue;
      }
      if (getValue(ix, iy, v))
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
    LOG(ERROR) << "no data found in sdev_xy";
    return _missing;
  }

}

//----------------------------------------------------------------
void GridAlgs::median2(const int xw, const int yw, double bin_min,
		      double bin_max, double bin_delta)
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
void GridAlgs::speckle(const int xw, int yw, double bin_min, double bin_max,
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
  
  m = _speckle(*this, xw, yw, F, G);
  G.getXy(x, y);
  setValue(x, y, m);

  while (G.increment())
  {
    m = _speckle(*this, xw, yw, F, G);
    G.getXy(x, y);
    setValue(x, y, m);
  }
}

//----------------------------------------------------------------
void GridAlgs::speckleInterest(const int xw, const int yw,
			       double bin_min, double bin_max,
			       double bin_delta, const FuzzyF &fuzzyDataDiff,
			       const FuzzyF &fuzzyCountPctDiff)
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
  
  m = _speckleInterest(*this, xw, yw, fuzzyDataDiff, fuzzyCountPctDiff, F, G);
  G.getXy(x, y);
  setValue(x, y, m);

  while (G.increment())
  {
    m = _speckleInterest(*this, xw, yw, fuzzyDataDiff, fuzzyCountPctDiff, F, G);
    G.getXy(x, y);
    setValue(x, y, m);
  }
}

#ifdef NOTYET
void GridAlgs::threadedSpeckleInterest(const int xw, const int yw,
				       double bin_min, double bin_max,
				       double bin_delta,
				       const FuzzyF &fuzzyDataDiff,
				       const FuzzyF &fuzzyCountPctDiff,
				       int nthread)
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
  
  m = _speckleInterest(*this, xw, yw, fuzzyDataDiff, fuzzyCountPctDiff, F, G);
  G.getXy(x, y);
  setValue(x, y, m);

  while (G.increment())
  {
    m = _speckleInterest(*this, xw, yw, fuzzyDataDiff, fuzzyCountPctDiff, F, G);
    G.getXy(x, y);
    setValue(x, y, m);
  }
}
#endif

//----------------------------------------------------------------
void GridAlgs::medianNoOverlap(const int xw, const int yw, 
			       const double bin_min,
			       const double bin_max, const double bin_delta,
			       const bool allow_any_data)
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
void GridAlgs::sdev2Old(const int xw, const int yw)
{
  // make a copy of the input grid..
  GridAlgs tmp(*this);

  // for each point, set value to standard deviation in a xw by yw window around
  // the point
  for (int y=0; y<_ny; y++)
  {
    for (int x=0; x<_nx; x++)
    {
      _data[y*_nx + x] = tmp._sdev(x, y, xw, yw);
    }
  }
}

//---------------------------------------------------------------------------
void GridAlgs::sdevThreaded(const int sx, const int sy, int numThread)
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

//---------------------------------------------------------------------------
void GridAlgs::textureXThreaded(const int sx, const int sy, int numThread)
{
  GridAlgThreads *thread = new GridAlgs::GridAlgThreads();
  thread->init(numThread, false);

  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::TEXTURE_X,
					  sx, sy, iy, this, tmp);
    thread->thread(iy, (void *)info);
  }
  thread->waitForThreads();
  delete thread;
  *this = tmp;
}

//---------------------------------------------------------------------------
void GridAlgs::textureYThreaded(const int sx, const int sy, int numThread)
{
  GridAlgThreads *thread = new GridAlgs::GridAlgThreads();
  thread->init(numThread, false);

  GridAlgs tmp(*this);
  for (int iy=0; iy<_ny; ++iy)
  {
    GridAlgsInfo *info = new GridAlgsInfo(GridAlgsInfo::TEXTURE_Y,
					  sx, sy, iy, this, tmp);
    thread->thread(iy, (void *)info);
  }
  thread->waitForThreads();
  delete thread;
  *this = tmp;
}

//----------------------------------------------------------------
void GridAlgs::sdev2(const int xw, const int yw)
{
  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // make a copy of the local object
  GridAlgs tmp(*this);

  // consider this: sdev = sqrt(sum (xi-xbar)**2/N)
  //                     = sum(xi**2 ) -2*xbar*sum(xi) + sum(xbar**2)
  //                xbar = sum(xi)/N
  //                sdev = sqrt(Z/N)
  //  where
  //           Z = sum(xi**2) -2*sum(xi)*xbar + N*xbar*xbar
  //             = sum(xi**2) -2*sum(xi)*sum(xi)/N + N*sum(xi)*sum(xi)/(N*N)
  //             = sum(xi**2) -sum(xi)*sum(xi)/N
  //             = (N*sum(xi**2) - (sum(xi))**2)/N
  //    
  // we can therefore do this the fast way as follows. Let
  //   A = sum(xi**2)
  //   B = sum(xi)
  // 
  //   Z = (N*A - B*B)/N
  //   sdev = sqrt(Z/N) = sqrt(N*A-B*B)/N
  //
  // When we increment the G and get old/new points, we can say
  //   A(k+1) = A(k) - sum(xi**2)old + sum(xi**2)new
  //   B(k+1) = B(k) - sum(xi)old + sum(xi)new
  //   N(k+1) = N(k) - number removed + number added
  //
  // which shows that this can be done using the 'fast' method.
  
  double A=0, B=0, N=0;

  // do this computation writing to local object
  _sdev2Final(xw, yw, tmp, G, A, B, N);
  while (G.increment())
  {
    _sdev2Final(xw, yw, tmp, G, A, B, N);
  }
}

//----------------------------------------------------------------
void GridAlgs::sdevNoOverlap(const int xw, const int yw)
{
  // make a copy of the local object
  GridAlgs tmp(*this);

  // Note: sdev = sqrt(sum (xi-xbar)**2/N)
  //            = sum(xi**2 ) -2*xbar*sum(xi) + sum(xbar**2)
  //       xbar = sum(xi)/N
  //    

  for (int y=0; y<_ny; y += yw)
  {
    for (int x=0; x<_nx; x += xw)
    {
      double xbar = tmp._meanInBox(x, y, xw, yw);
      double sdev = tmp._sdevInBox(x, y, xw, yw, xbar);
      _fillBox(x, y, xw, yw, sdev);
    }
  }
}

//----------------------------------------------------------------
void GridAlgs::texture2X(const int xw, const int yw)
{
  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // make a copy of the local object
  GridAlgs tmp(*this);

  double A=0.0;
  double N=0.0;

  // do this computation writing to local object
  _textureXNew(xw, yw, tmp, G, A, N);
  while (G.increment())
  {
    _textureXNew(xw, yw, tmp, G, A, N);
  }
}

//----------------------------------------------------------------
void GridAlgs::texture2Y(const int xw, const int yw)
{
  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // make a copy of the local object
  GridAlgs tmp(*this);

  double A=0.0;
  double N=0.0;

  // do this computation writing to local object
  _textureYNew(xw, yw, tmp, G, A, N);
  while (G.increment())
  {
    _textureYNew(xw, yw, tmp, G, A, N);
  }
}

//----------------------------------------------------------------
void GridAlgs::smooth2(const int xw, const int yw)
{
  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // make a copy of the local object
  GridAlgs tmp(*this);

  double A=0.0, N=0.0;

  // do this computation writing to local object
  _smooth2(xw, yw, tmp, G, A, N);
  while (G.increment())
  {
    _smooth2(xw, yw, tmp, G, A, N);
  }
}

//----------------------------------------------------------------
void GridAlgs::smooth2NoMissing(const int xw, const int yw)
{
  // make an object to loop through the grid
  Grid2dLoop G(_nx, _ny);

  // make a copy of the local object
  GridAlgs tmp(*this);

  double A=0.0, N=0.0;
  int nmissing = 0;

  // do this computation writing to local object
  _smooth2NoMissing(xw, yw, tmp, G, A, N, nmissing);
  while (G.increment())
  {
    _smooth2NoMissing(xw, yw, tmp, G, A, N, nmissing);
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
      if (!isMissing(x,y))
      {
        getValue(x, y, v1);
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
      if (!isMissing(x,y))
      {
	getValue(x, y, v1);
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
void GridAlgs::xMaxForAllY(const int nx)
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
void GridAlgs::xAverageForAllY(const int nx)
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
void GridAlgs::xMaxForAllY(const int nx, const double min_v)
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
void GridAlgs::xAverageForAllY(const int nx, const double min_v)
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
void GridAlgs::xPcntGeForAllY(const int nx, const double min_v,
			      const double min_pct, int &x0, int &x1)
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
void GridAlgs::clearNear(const int x, const int y, const int n)
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
void GridAlgs::shiftX(const int npt)
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
void GridAlgs::maskRange(const Grid2d &mask, const double low,
			 const double high)
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
				      const double  mask_value,
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

//----------------------------------------------------------------
void GridAlgs::belowThresholdToMissing(const double thresh)
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

//----------------------------------------------------------------
double GridAlgs::_meanInBox(const int x0, const int y0, const int nx,
			    const int ny) const
{
  double v=0.0, n=0.0;

  for (int y=y0; y<y0+ny; ++y)
  {
    if (y >= 0 && y < _ny)
    {
      for (int x=x0; x<x0+nx; ++x)
      {
	if (x >= 0 && x < _nx)
	{
	  double vxy;
	  if (getValue(x, y, vxy))
	  {
	    ++n;
	    v += vxy;
	  }
	}
      }
    }
  }
  if (n > 0.0)
  {
     return v/n;
  }
  else
  {
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::_sdevInBox(const int x0, const int y0, const int nx,
			    const int ny, const double xbar) const
{
  // Note: sdev = sqrt(sum (xi-xbar)**2/N)
  double v=0.0, n=0.0;

  for (int y=y0; y<y0+ny; ++y)
  {
    if (y >= 0 && y < _ny)
    {
      for (int x=x0; x<x0+nx; ++x)
      {
	if (x >= 0 && x < _nx)
	{
	  double vxy;
	  if (getValue(x, y, vxy))
	  {
	    ++n;
	    v += (vxy-xbar)*(vxy-xbar);
	  }
	}
      }
    }
  }
  if (n > 0.0)
  {
    return sqrt(v/n);
  }
  else
  {
    return _missing;
  }

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
double GridAlgs::_dilate(const int x, const int y, const int xw,
			const int yw) const
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
double GridAlgs::_dilateOneValue(const double value, const int x,
				const int y, const int xw, const int yw) const
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
double GridAlgs::_median(const int x, const int y, const int xw,
			const int yw) const
{
  vector <double> aList;
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
        aList.push_back(v);
      }
    }
  }

  if (!aList.empty())
  {
    int ind = aList.size() / 2;
    nth_element(aList.begin(), aList.begin()+ind, aList.end());
    return aList[ind];
  }
  else
  {
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::_sdev(const int x, const int y, const int xw,
		       const int yw, bool needHalf) const
{
  double A=0,Q=0,N=0;
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
	// from wikipedia
        N+=1;
        Q = Q+((N-1)/N)*(v-A)*(v-A);
        A = A+(v-A)/N;
      }
    }
  }

  double maxbad;
  if (needHalf)
  {
    maxbad = static_cast<double>(xw*yw)/2.0;
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
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::_sdev2(const int x, const int y, const int xw,
		       const int yw) const
{
  double mean=0.0;
  double n = 0.0;
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
        n+=1;
	mean += v;
      }
    }
  }
  if (n > 0)
  {
    mean /= n;
  }
  else
  {
    return _missing;
  }

  double sdev=0.0;
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
	double d = (v-mean);
	sdev += d*d;
      }
    }
  }
  return sqrt(sdev/n);
}

//----------------------------------------------------------------
double GridAlgs::_textureX(const int x, const int y, const int xw,
			   const int yw, bool needHalf) const
{
  double A=0.0;
  int n = 0;

  int maxbad;
  if (needHalf)
  {
    maxbad = xw*yw/2;
  }
  else
  {
    maxbad = 0;
  }

  for (int iy=y-yw; iy<=y+yw; ++iy)
  {
    if (iy-1 < 0 || iy-1 >= _ny)
    {
      continue;
    }
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
      double v1,v2;
      if (getValue(ix, iy, v1) && getValue(ix,iy-1,v2))
      {
	A = A+(v1-v2)*(v1-v2);
	n++;
      }
    }
  }
  if (n > maxbad)
  {
    return A/static_cast<double>(n);//yw*xw-1);
  }
  else
  {
    return _missing;
  }
}

//----------------------------------------------------------------
double GridAlgs::_textureY(const int x, const int y, const int xw,
			   const int yw, bool needHalf) const
{
  double A=0.0;
  int n = 0;

  int maxbad;
  if (needHalf)
  {
    maxbad = xw*yw/2;
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
    for (int ix=x-xw; ix<=x+xw; ++ix)
    {
      if (ix-1 < 0 || ix-1 >= _nx)
      {
	continue;
      }
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      double v1,v2;
      if (getValue(ix, iy, v1) && getValue(ix-1,iy,v2))
      {
	A = A+(v1-v2)*(v1-v2);
	n++;
      }
    }
  }
  if (n > maxbad)
  {
    return A/static_cast<double>(n);//yw*xw-1);
  }
  else
  {
    return _missing;
  }
}

//----------------------------------------------------------------
void GridAlgs::_fillEdge(const int xw, const int yw, const double value)
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
bool GridAlgs::_fillHole(const int n, const int ix, const int iy)
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
bool GridAlgs::_fillHhole(const int n, const int ix, const int iy)
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
bool GridAlgs::_fillVhole(const int n, const int ix, const int iy)
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
// compute and store standard deviation using 'fast' method into local object
// use grid of means Xbar and counts N
// A = previous state sum of values squared, updated on exit.
// B = previous state sum of values, updated on exit.
//
void GridAlgs::_sdev2Final(const int xw, const int yw, const Grid2d &tmp,
			  Grid2dLoop &G, double &A, double &B, double &N)
{
  int x, y;
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get the value
  G.getXy(x, y);

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // remove all old squared values and non-squared from A and B, and dec. N
  double vi;
  for (i=oldv.begin(); i!=oldv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, vi))
    {
      A -= (vi*vi);
      B -= vi;
      N --;
    }
  }
  // add in squares and non-squares to A and B, inc. N
  for (i=newv.begin(); i!=newv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, vi))
    {
      A += (vi*vi);
      B += vi;
      N ++;
    }
  }

  // compute numerator from that, and use to build result
  if (N > xw*yw/2)
  {
    double numerator = N*A-B*B;
    _data[y*_nx + x] = sqrt(numerator)/N;
  }
  else
  {
    _data[y*_nx + x] = _missing;
  }
}

//----------------------------------------------------------------
void GridAlgs::_textureXNew(const int xw, const int yw, const Grid2d &tmp,
			   Grid2dLoop &G, double &A, double &N)
{
  int x, y;
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get the value
  G.getXy(x, y);

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // remove all old contrubutions to A
  double v1, v2;
  int ix, iy;
  for (i=oldv.begin(); i!=oldv.end(); ++i)
  {
    ix = i->first;
    iy = i->second;
    if (iy-1 >= 0)
    {
      if (tmp.getValue(ix, iy, v1) && tmp.getValue(ix,iy-1,v2))
      {
	A -= (v1-v2)*(v1-v2);
	N --;
      }
    }
  }

  // add in new contributions to A
  for (i=newv.begin(); i!=newv.end(); ++i)
  {
    ix = i->first;
    iy = i->second;
    if (iy-1 >= 0)
    {
      if (tmp.getValue(ix, iy, v1) && tmp.getValue(ix,iy-1,v2))
      {
	A += (v1-v2)*(v1-v2);
	N ++;
      }
    }
  }
  if (N > xw*yw/2)
  {
    _data[y*_nx + x] = A/N;
  }
  else
  {
    _data[y*_nx + x] = _missing;
  }
}

//----------------------------------------------------------------
void GridAlgs::_textureYNew(const int xw, const int yw, const Grid2d &tmp,
			   Grid2dLoop &G, double &A, double &N)
{
  int x, y;
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get the value
  G.getXy(x, y);

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // remove all old contrubutions to A
  double v1, v2;
  int ix, iy;
  for (i=oldv.begin(); i!=oldv.end(); ++i)
  {
    ix = i->first;
    iy = i->second;
    if (ix-1 >= 0)
    {
      if (tmp.getValue(ix, iy, v1) && tmp.getValue(ix-1,iy,v2))
      {
	A -= (v1-v2)*(v1-v2);
	N --;
      }
    }
  }

  // add in new contributions to A
  for (i=newv.begin(); i!=newv.end(); ++i)
  {
    ix = i->first;
    iy = i->second;
    if (ix-1 >= 0)
    {
      if (tmp.getValue(ix, iy, v1) && tmp.getValue(ix-1,iy,v2))
      {
	A += (v1-v2)*(v1-v2);
	N ++;
      }
    }
  }

  if (N >  xw*yw/2)
  {
    _data[y*_nx + x] = A/N;
  }
  else
  {
    _data[y*_nx + x] = _missing;
  }
}

//----------------------------------------------------------------
void GridAlgs::_smooth2(const int xw, const int yw, const Grid2d &tmp,
		       Grid2dLoop &G, double &A, double &N)
{
  int x, y;
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get the value
  G.getXy(x, y);

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // remove all old contrubutions to A
  double v;
  for (i=oldv.begin(); i!=oldv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, v))
    {
      A -= v;
      N --;
    }
  }

  // add in new contributions to A
  for (i=newv.begin(); i!=newv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, v))
    {
      A += v;
      N ++;
    }
  }

  if (N > xw*yw/2)
  {
    _data[y*_nx + x] = A/N;
  }
  else
  {
    _data[y*_nx + x] = _missing;
  }
}

//----------------------------------------------------------------
void GridAlgs::_smooth2NoMissing(const int xw, const int yw,
				const Grid2d &tmp,
				Grid2dLoop &G, double &A, double &N,
				int &nmissing)
{
  int x, y;
  vector<pair<int,int> >  newv, oldv;
  vector<pair<int,int> >::iterator i;

  // get the value
  G.getXy(x, y);

  // get old and new points based on state
  newv = G.newXy(xw, yw);
  oldv = G.oldXy(xw, yw);
  
  // remove all old contrubutions to A
  double v;
  for (i=oldv.begin(); i!=oldv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, v))
    {
      A -= v;
      N --;
    }
    else
    {
      nmissing --;
    }
  }

  // add in new contributions to A
  for (i=newv.begin(); i!=newv.end(); ++i)
  {
    if (tmp.getValue(i->first, i->second, v))
    {
      A += v;
      N ++;
    }
    else
    {
      nmissing++;
    }
  }

  if (nmissing <= 0)
  {
    _data[y*_nx + x] = A/N;
  }
  else
  {
    _data[y*_nx + x] = _missing;
  }
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
