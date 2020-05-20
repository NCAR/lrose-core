/**
 * @file GridExpand.cc
 */

#include "GridExpand.hh"
#include <toolsa/LogStream.hh>
#include <cmath>

//----------------------------------------------------------------
GridExpand::GridExpand(int nptX) : _nptX(nptX)
{
}

//----------------------------------------------------------------
GridExpand::~GridExpand()
{
}

//----------------------------------------------------------------
void GridExpand::update(const Grid2d &g, const MdvxProj &proj,
			Grid2d &e)
{
  int nx = g.getNx();
  int ny = g.getNy();

  double dx = proj.xGrid2km(1.0);
  double dy = proj.yGrid2km(1.0);  /// actualy degrees?
  printf("dy=%lf\n", dy);
  double maxKm = dx*(double)_nptX;
  printf("MaxKM from x = %lf\n", maxKm);
  
  for (int x = 0; x < nx; x++)
  { 
    // figure out arc length at this distance
    double R = (double)x*dx;
    double yKmPerPixel = R*3.14159/180.0*dy;
    int nptY = maxKm/yKmPerPixel;
    // printf("At radius = %lf, yKmPerPixel=%lf   nptY=%d\n", R,
    // 	   yKmPerPixel, nptY);
    if (nptY > _nptX)
    {
      // to prevent too much near radar
      nptY = _nptX;
      // printf("Adjusting nptY to %d\n", nptY);
    }
    
    for (int y = 0; y < ny; y++)
    {
      double v;
      if (g.getValue(x, y, v))
      {
	double v2;
	if (e.getValue(x,y, v2))
	{
	  if (v2 < v)
	  {
	    e.setValue(x, y, v);
	  }
	}
	else
	{
	  e.setValue(x, y, v);
	}
	for (int iy=y-nptY; iy<=y+nptY; ++iy)
	{
	  if (iy < 0 || iy >= ny)
	  {
	    continue;
	  }
	  for (int ix=x-_nptX; ix <= x+_nptX; ++ix)
	  {
	    if (ix < 0 || ix >= nx)
	    {
	      continue;
	    }
	    double v3;
	    if (e.getValue(ix, iy, v3))
	    {
	      if (v3 < v)
	      {
		e.setValue(ix, iy, v3);
	      }
	    }
	    else
	    {
	      e.setValue(ix, iy, v);
	    }
	  }
	}
      }
    }
  }
}
