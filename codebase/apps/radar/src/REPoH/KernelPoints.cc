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
 * @file KernelPoints.cc
 */
#include "KernelPoints.hh"
#include "KernelTemplate.hh"
#include "RepohParams.hh"
#include <Mdv/MdvxProj.hh>
#include <toolsa/LogStream.hh>

/*----------------------------------------------------------------*/
// mean linearized value at a set of points in a grid
static double _mean_linear(const Grid2d &g, const PointList &pts)
{
  double ave = 0.0, n=0.0;

  for (int i=0; i<(int)pts.size(); ++i)
  {
    double v;
    int x = pts.ithPoint(i).getX();
    int y = pts.ithPoint(i).getY();
    if (g.getValue(x, y, v))
    {
      double v2 = v/10;
      v2 = pow(10,v2); 
      ave += v2; 
      n++;
    }
  }
  if (n > 0)
    ave /= n;
  return ave;
}


/*----------------------------------------------------------------*/
// mean of linearized values at a set of points, put back into log space
static double _mean_linear_adjusted(const Grid2d &g, const PointList &pts)

{
  double ave = _mean_linear(g, pts);
  ave = 10.0*log10(ave);
  return ave;
}

/*----------------------------------------------------------------*/
// difference between min and max values
static double _diff(const Grid2d &g, const PointList &pts)
{
  double min=0, max=0;

  if (pts.min(g, min) && pts.max(g, max))
  {
    return max - min;
  }
  else
  {
    LOG(WARNING) << " NO data ";
    return 0.0;
  }
}


/*----------------------------------------------------------------*/
KernelPoints::KernelPoints(void)
{
  _bigEnough = false;
}

/*----------------------------------------------------------------*/
KernelPoints::KernelPoints(const Grid2d &mask, const Grid2d &omask,
			   const MdvxProj &proj, const Point &centerPt,
			   bool atRadar, bool isFar, const RepohParams &params)
{
  Mdvx::coord_t coord = proj.getCoord();
  _pts = PointList(coord.nx, coord.ny);
  _outPts = PointList(coord.nx, coord.ny);
  _pts.append(centerPt);

  if (atRadar)
  {
    // at the origin, must be _ok
    _bigEnough = true;
  }
  else
  {
    // not at origin, add in all the points that define the kernel
    int nx, ny;
    nx = mask.getNx();
    ny = mask.getNy();
    KernelTemplate T(nx, ny, centerPt, isFar);
    T.add_kernel_points(mask, params.max_kernel_size, *this);
    T.add_kernel_outside_points(omask, params.max_kernel_size, *this);

    // define _ok as when the resultant set of points is big enough
    _bigEnough = (npt() >= params.min_kernel_size &&
		  nptOutside() >= params.min_kernel_size);
  }
}

/*----------------------------------------------------------------*/
KernelPoints::~KernelPoints()
{
}

/*----------------------------------------------------------------*/
void KernelPoints::print(void) const
{
  printf("pts:\n");
  _pts.print();
  printf("outside pts:\n");
  _outPts.print();
}

/*----------------------------------------------------------------*/
std::string KernelPoints::sprintDebug(double km_per_gate) const
{
  string ret = "";
  char buf[1000];
  for (int i=0; i< (int)_pts.size(); ++i)
  {
    double v = (double)_pts.ithPoint(i).getX()*km_per_gate;
    //[i].first*km_per_gate;
    sprintf(buf, "%10.5f ", v);
    ret += buf;
  }
  return ret;
}

/*----------------------------------------------------------------*/
void KernelPoints::toGrid(Grid2d &gr, double v,  bool outside) const
{
  if (outside)
  {
    _outPts.toGrid(gr, v);
  }
  else
  {
    _pts.toGrid(gr, v);
  }
}

/*----------------------------------------------------------------*/
double KernelPoints::meanLinearAdjusted(const Grid2d &grid, bool outside) const
{
  if (outside)
  {
    return _mean_linear_adjusted(grid, _outPts);
  }
  else
  {
    return _mean_linear_adjusted(grid, _pts);
  }
}

/*----------------------------------------------------------------*/
double KernelPoints::meanLinear(const Grid2d &grid, bool outside) const
{
  if (outside)
  {
    return _mean_linear(grid, _outPts);
  }
  else
  {
    return _mean_linear(grid, _pts);
  }
}

/*----------------------------------------------------------------*/
double KernelPoints::dataRange(const Grid2d &grid, bool outside) const
{
  if (outside)
  {
    return _diff(grid, _outPts);
  }
  else
  {
    return _diff(grid, _pts);
  }
}

/*----------------------------------------------------------------*/
void KernelPoints::removeOutliers(const Grid2d &data, double diffThresh,
				  int minSize)
{
  // filter outliers in dbz_diff data from pts data 
  _pts.removeOutlierValuedPoints(data, diffThresh, minSize);
}

/*----------------------------------------------------------------*/
bool KernelPoints::enoughPoints(int min_kernel_size) const
{
  return _pts.size() >= min_kernel_size;
}

/*----------------------------------------------------------------*/
void KernelPoints::printData(FILE *fp, const Grid2d &g) const
{
  for (int i=0; i< (int)_pts.size(); ++i)
  {
    double v;
    int ix = _pts.ithPoint(i).getX();
    int iy = _pts.ithPoint(i).getY();
    if (g.getValue(ix, iy, v))
      fprintf(fp, "%10.5f ", v);
    else
      fprintf(fp, "%10.5f ", -99.9999);
  }
  fprintf(fp, "\n");
}
