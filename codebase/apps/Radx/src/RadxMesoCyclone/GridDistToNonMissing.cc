/**
 * @file GridDistToNonMissing.cc
 */

#include "GridDistToNonMissing.hh"
#include <toolsa/LogStream.hh>
#include <cmath>

//----------------------------------------------------------------
GridDistToNonMissing::GridDistToNonMissing(int maxSearch, int searchScale) :
  _maxSearch(maxSearch), _searchScale(searchScale), _nx(0), _ny(0)
{
}

//----------------------------------------------------------------
GridDistToNonMissing::~GridDistToNonMissing()
{
}

//----------------------------------------------------------------
void GridDistToNonMissing::update(const Grid2d &g, const MdvxProj &proj)
{
  proj.latlon2xyIndex(42.9512, -107.1605, _xDebugBad, _yDebugBad);
  proj.latlon2xyIndex(43.464, -105.3169, _xDebugData, _yDebugData);
  
  int nx = g.getNx();
  int ny = g.getNy();
  if (nx != _nx || ny != _ny)
  {
    _nx = nx;
    _ny = ny;
    _xIndex = Grid2d("xIndex", nx, ny, MISSING);
    _yIndex = Grid2d("yIndex", nx, ny, MISSING);
    _rebuild(g, proj);
  }
  else
  {
    if (_missingChanged(g))
    {
      _rebuild(g, proj);
    }
  }
}

//----------------------------------------------------------------
bool GridDistToNonMissing::distanceToNonMissing(const MdvxProj &proj,
						const Grid2d &data,
						Grid2d &distOut,
						Grid2d &valOut)
{      
  proj.latlon2xyIndex(42.9512, -107.1605, _xDebugBad, _yDebugBad);
  proj.latlon2xyIndex(43.464, -105.3169, _xDebugData, _yDebugData);
  
  // check dimensions for consistency
  if (data.getNdata() != distOut.getNdata() ||
      data.getNdata() != valOut.getNdata())
  {
    LOG(ERROR) << "Dimensions inconsistent";
    return false;
  }

  // update if update is needed
  update(data, proj);

  // max radius is in terms of number of gridpoints radially
  // azimuthal distances will be scaled approximately based on current radius
  double maxRadius = _maxSearch*proj.xGrid2km(1.0);

  // At each grid point x,y find and record minimum distances to non-missing
  // data and the non missing data value  
  for (int j = 0; j < _ny; j++)
  {
    for (int i = 0; i < _nx; i++)
    { 
      bool debugBad = (i >= _xDebugBad-1 && i<= _xDebugBad + 1 &&
		       j >= _yDebugBad-1 && j<= _yDebugBad + 1);
      bool debugData = (i >= _xDebugData-1 && i<= _xDebugData + 1 &&
		   j >= _yDebugData-1 && j<= _yDebugData + 1);
      if (debugBad || debugData)
      {
	printf("Here\n");
      }
      // Taxicab metric distance to non-missing data from (i,j)
      double minDist = maxRadius;   

      // Value of data at minimum distance
      double closeVal;

      // Flag to indicate successful search
      bool distFound = false;
      
      // Check right away if point is not missing or bad
      // since we need not go further and distance = 0 at such points
      if (!data.isMissing(i, j))
      { 
	distFound = true;
	minDist = 0.0;
	data.getValue(i, j, closeVal);
      }
      else
      {
	int nearI, nearJ;
	if ((distFound = nearestPoint(i, j, nearI, nearJ)))
	{
	  double di = fabs(nearI - i);
	  double dj = fabs(nearJ - j);
	  // here try scaling dj based on i.
	  double R = (double)i*proj.xGrid2km(1.0);
	  // turn this into an arc length, assuming 1 degree data
	  // and then scale to # of gates
	  dj = dj*3.14159/180.0*R/proj.xGrid2km(1.0);
	  if (dj > di)
	  {
	    di = dj;
	  }
	  minDist = di*proj.xGrid2km(1.0);
	  if (!data.getValue(nearI, nearJ, closeVal))
	  {
	    LOG(WARNING) << "Unexpected missing where not expected ("
			 << nearI << "," << nearJ << ")";
	    distFound = false;
	  }
	}
      }
      
      if (!distFound)
      {
	// At points where no nearby data is non-missing, set result to max
	distOut.setValue(i, j, maxRadius);
	valOut.setMissing(i, j);
      }
      else
      {
	// Set the minimum distance to non missing data and the value 
	distOut.setValue(i, j, minDist);
	valOut.setValue(i, j, closeVal);
      }
    } // End for
  }// End for

  return true;
}

//----------------------------------------------------------------
bool GridDistToNonMissing::nearestPoint(int x, int y, int &nearX,
					int &nearY) const
{
  double dx, dy;
  if (_xIndex.getValue(x, y, dx) && _yIndex.getValue(x, y, dy))
  {
    if (dx == HAS_DATA)
    {
      nearX = x;
      nearY = y;
      return true;
    }
    else
    {
      nearX = static_cast<int>(dx);
      nearY = static_cast<int>(dy);
      return true;
    }
  }
  else
  {
    // nothing
    return false;
  }
}

//----------------------------------------------------------------
bool GridDistToNonMissing::_missingChanged(const Grid2d &g) const
{
  int diff = 0;
  for (int i=0; i<g.getNdata(); ++i)
  {
    // is the new grid data missing at i?
    bool newMissing = g.isMissing(i);

    // the state says data was missing at i if either an index is set
    // indicating an offset 
    // (the value is not missing and is not HAS_DATA) or the
    // index is not set (the point has missing data and is out of the
    // max search distance)
    bool oldMissing;
    double v;
    if (_xIndex.getValue(i, v))
    {
      oldMissing = v != HAS_DATA;
    }
    else
    {
      oldMissing = true;
    }
    if (oldMissing != newMissing)
    {
      ++diff;
    }
  }
  if (diff > 0)
  {
    LOG(DEBUG_VERBOSE) <<  "Rebuild because " << diff << "  points different";
    return true;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Do not Rebuild because 0 points different";
    return false;
  }
}

//----------------------------------------------------------------
void GridDistToNonMissing::_rebuild(const Grid2d &g, const MdvxProj &proj)
{
  // initialize to MISSING everywhere (MISSING is the missing value)
  _xIndex.setAllMissing();
  _yIndex.setAllMissing();

  // create a copy of the input grid  (gradually to be eroded away)
  Grid2d s(g);

  for (int r=_searchScale; r<_maxSearch; r += _searchScale)
  {
    int nmissing = s.getNdata() - s.numGood();
    LOG(DEBUG_VERBOSE) << "Checking r=" << r << ", nmissing=" << nmissing;
    int totalNew=0;
    for (int y=0; y<_ny; ++y)
    {
      for (int x=0; x<_nx; ++x)
      {
	bool debugBad = (x >= _xDebugBad-1 && x<= _xDebugBad + 1 &&
			 y >= _yDebugBad-1 && y<= _yDebugBad + 1);
	bool debugData = (x >= _xDebugData-1 && x<= _xDebugData + 1 &&
			  y >= _yDebugData-1 && y<= _yDebugData + 1);
	if (!s.isMissing(x, y))
	{
	  if (debugBad || debugData)
	  {
	    printf("HERE\n");
	  }
	  int nnew = 0;
	  nnew = _rebuild1(r, x, y, g, proj);
	  totalNew += nnew;
	  if (nnew == 0)
	  {
	    // at larger search scales, we will want to skip over this point
	    // because the point was not used at a smaller scale.  This makes
	    // things more efficient.
	    s.setMissing(x, y);
	  }
	}
      }
    }
    LOG(DEBUG_VERBOSE) << "R=" << r << " had " << totalNew << " new";
  }

  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
	bool debugBad = (x >= _xDebugBad-1 && x<= _xDebugBad + 1 &&
			 y >= _yDebugBad-1 && y<= _yDebugBad + 1);
	bool debugData = (x >= _xDebugData-1 && x<= _xDebugData + 1 &&
			  y >= _yDebugData-1 && y<= _yDebugData + 1);
      if (!g.isMissing(x, y))
      {
	if (debugBad || debugData)
	{
	  printf("HERE\n");
	}
	_xIndex.setValue(x, y, HAS_DATA);
	_yIndex.setValue(x, y, HAS_DATA);
      }
    }
  }
}

//----------------------------------------------------------------
int GridDistToNonMissing::_rebuild1(int r, int x, int y, const Grid2d &g,
				    const MdvxProj &proj)
{
  // x,y is the point with data

  int nnew = 0;

  // get distance between gates
  double rx = proj.xGrid2km(1.0);

  // set bounds on how far to look in y (azimuth) based on x (range)
  double R = (double)x*proj.xGrid2km(1.0);

  // arc length between azimuths is around R*theta assumed 1.0 degree
  double ry = R*3.14159/180.0;
  int iry = (int)((double)r*rx/ry);

  if (iry > r)  iry = r;  // to prevent blowups
  if (iry <= 0) iry = 1;  // to prevent infinite loops

  int ysearchscale = (int)((double)_searchScale*rx/ry);

  // to prevent infinite loops
  if (ysearchscale <= 0) ysearchscale = 1;

  //
  // Search horizontal edges of box surrounding point (x, y)
  // with box lengths 2r
  for (int iy=y-iry; iy<=y+iry; iy += 2*iry)
  {
    if (iy < 0 || iy >= _ny)
    {
      continue;
    }
    for (int ix = x-r; ix<=x+r; ix += _searchScale)
    {
      if (ix >= 0 && ix < _nx)
      {
	if (g.isMissing(ix, iy))  
	{
	  if (_xIndex.isMissing(ix, iy))
	  {
	    bool debugBad = (ix >= _xDebugBad-1 && ix<= _xDebugBad + 1 &&
			     iy >= _yDebugBad-1 && iy<= _yDebugBad + 1);
	    bool debugData = (ix >= _xDebugData-1 && ix <= _xDebugData + 1 &&
			      iy >= _yDebugData-1 && iy<= _yDebugData + 1);
	    if (debugBad || debugData)
	    {
	      printf("HERE\n");
	    }
	    // not yet set, set now
	    _xIndex.setValue(ix, iy, x);
	    _yIndex.setValue(ix, iy, y);
	    ++nnew;
	  }
	}
      }
    }
    //
    // Search vertical edges of box surrounding point (i,j)
    // with box lengths 2r
    // 
    for (int ix=x-r; ix<=x+r; ix+= 2*r)
    {
      if (ix < 0 || ix >= _nx)
      {
	continue;
      }
      for (int iy=y-iry; iy<=y+iry; iy += ysearchscale)
      {
	if (iy >= 0 && iy < _ny)
	{
	  if (g.isMissing(ix, iy))
	  {
	    if (_xIndex.isMissing(ix, iy))
	    {
	    bool debugBad = (ix >= _xDebugBad-1 && ix<= _xDebugBad + 1 &&
			     iy >= _yDebugBad-1 && iy<= _yDebugBad + 1);
	    bool debugData = (ix >= _xDebugData-1 && ix <= _xDebugData + 1 &&
			      iy >= _yDebugData-1 && iy<= _yDebugData + 1);
	    if (debugBad || debugData)
	    {
	      printf("HERE\n");
	    }
	      // not yet set, set now
	      _xIndex.setValue(ix, iy, x);
	      _yIndex.setValue(ix, iy, y);
	      ++nnew;
	    }
	  }
	}
      }
    }
  }
  return nnew;
}

