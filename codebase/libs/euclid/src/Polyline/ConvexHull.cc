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
#include <euclid/ConvexHull.hh>
#include <algorithm>

/*----------------------------------------------------------------*/
ConvexHull::ConvexHull(const std::vector<int> x, const std::vector<int> y)
{
  if (x.size() < 2)
  {
    return;
  }
  // note that the very last point might be equal to the 0'th point, if so we
  // ignore it.
  int n = static_cast<int>(x.size());
  if (x[0] == x[n-1] && y[0] == y[n-1])
  {
    n = static_cast<int>(x.size())-1;
  }
  else
  {
    n = static_cast<int>(x.size());
  }

  // build the points
  for (int i=1; i<n; ++i)
  {
    _xyBuild.push_back(ConvexHullPoint(x[i], y[i], x[0], y[0]));
  }


  // sort the list using isLeft
  sort(_xyBuild.begin(), _xyBuild.end(), ConvexHullPoint::isLeft);

  // now go through remove all but furthest for cases where they are equal
  // as regards left/right
  bool removed = true;
  while (removed)
  {
    removed = _removeFirstEqual();
  }
  
  // here we build up the convex hull actual points.
  _xyFinal.push_back(ConvexHullPoint1(_xyBuild[0]._x0, _xyBuild[0]._y0));
  
  for (int j=0; j<static_cast<int>(_xyBuild.size()); ++j)
  {
    bool ok = false;
    while (!ok)
    {
      int n = _xyFinal.size();
      if (n <= 1)
      {
	_xyFinal.push_back(ConvexHullPoint1(_xyBuild[j]._x, _xyBuild[j]._y));
	ok = true;
      }
      else
      {
	// make line segment out of top two points in stack (n-1, n-2)
	ConvexHullPoint lastLine(_xyFinal[n-1]._x, _xyFinal[n-1]._y, 
				 _xyFinal[n-2]._x, _xyFinal[n-2]._y);

	// make another line segment from n-2 to new point
	ConvexHullPoint newLine(_xyBuild[j]._x, _xyBuild[j]._y, 
				_xyFinal[n-2]._x, _xyFinal[n-2]._y);

	// if the new segment is to left of top two point semgent, push
	if (ConvexHullPoint::isLeft(lastLine, newLine))
	{
	  _xyFinal.push_back(ConvexHullPoint1(_xyBuild[j]._x, _xyBuild[j]._y));
	  ok = true;
	}
	else
	{
	  // pop the last point off of the stored points and repeat the loop
	  _xyFinal.pop_back();
	}
      }
    }
  }
  // add the initial point back in again for a closed polygon
  _xyFinal.push_back(ConvexHullPoint1(_xyBuild[0]._x0, _xyBuild[0]._y0));
}

/*----------------------------------------------------------------*/
ConvexHull::~ConvexHull(void)
{
}

/*----------------------------------------------------------------*/
void ConvexHull::print(void) const
{
  printf("Build:\n");
  printBuild();
  printf("Final:\n");
  printFinal();
}

/*----------------------------------------------------------------*/
void ConvexHull::printBuild(void) const
{
  for (size_t i=0; i<_xyBuild.size(); ++i)
  {
    _xyBuild[i].print();
  }
}

/*----------------------------------------------------------------*/
void ConvexHull::printFinal(void) const
{
  for (size_t i=0; i<_xyFinal.size(); ++i)
  {
    _xyFinal[i].print();
  }
}

/*----------------------------------------------------------------*/
std::vector<int> ConvexHull::getX(void) const
{
  std::vector<int> ret;
  if (_xyFinal.empty())
  {
    return ret;
  }
  ret.push_back(_xyFinal[0]._x);
  for (size_t i=0; i<_xyFinal.size(); ++i)
  {
    ret.push_back(_xyFinal[i]._x);
  }
  return ret;
}

/*----------------------------------------------------------------*/
std::vector<int> ConvexHull::getY(void) const
{
  std::vector<int> ret;
  if (_xyFinal.empty())
  {
    return ret;
  }
  ret.push_back(_xyFinal[0]._y);
  for (size_t i=0; i<_xyFinal.size(); ++i)
  {
    ret.push_back(_xyFinal[i]._y);
  }
  return ret;
}

/*----------------------------------------------------------------*/
bool ConvexHull::_removeFirstEqual(void)
{
  for (size_t i=0; i<_xyBuild.size(); ++i)
  {
    if (_removeEqual(i))
    {
      return true;
    }
  }
  return false;
}


/*----------------------------------------------------------------*/
bool ConvexHull::_removeEqual(size_t i)
{
  int p0 = i;
  int p1 = i;
  for (size_t j=i+1; j<_xyBuild.size(); ++j)
  {
    if (ConvexHullPoint::isEqual(_xyBuild[i], _xyBuild[j]))
    {
      p1 = j;
    }
    else
    {
      break;
    }
  }
  if (p0 == p1)
  {
    return false;
  }

  // figure out which one to keep
  int keep = p0;
  // find farthest one, the keeper, remove the others
  for (int k=p0+1; k<= p1; ++k)
  {
    if (ConvexHullPoint::isFurther(_xyBuild[k], _xyBuild[keep]))
    {
      keep = k;
    }
  }

  // want to remove everything from p0 to p1 except keep
  for (int i=p1; i>=p0; --i)
  {
    if (i != keep)
    {
      _xyBuild.erase(_xyBuild.begin()+i);
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
void ConvexHull::ConvexHullPoint::print(void) const
{
  printf("xy=(%d,%d)  xy0=(%d,%d)\n", _x, _y, _x0, _y0);
}
  
/*----------------------------------------------------------------*/
double ConvexHull::ConvexHullPoint::lengthSq(void) const
{
  return (_x-_x0)*(_x-_x0) + (_y-_y0)*(_y-_y0);
}

/*----------------------------------------------------------------*/
bool ConvexHull::ConvexHullPoint::isLeft(const ConvexHullPoint &p1,
					 const ConvexHullPoint &p2) 
{
  double which = ( (p1._x - p1._x0)*(p2._y - p2._y0) -
		   (p2._x - p2._x0)*(p1._y - p1._y0) );

  return which > 0;
}
  
/*----------------------------------------------------------------*/
bool ConvexHull::ConvexHullPoint::isEqual(const ConvexHullPoint &p1,
					  const ConvexHullPoint &p2)
{
  double which = ( (p1._x - p1._x0)*(p2._y - p2._y0) -
		   (p2._x - p2._x0)*(p1._y - p1._y0) );

  return which == 0;
}

/*----------------------------------------------------------------*/
bool ConvexHull::ConvexHullPoint::isFurther(const ConvexHullPoint &p1,
					    const ConvexHullPoint &p2)
{
  if (isEqual(p1, p2))
  {
    return p1.lengthSq() > p2.lengthSq();
  }
  else
  {
    return false;
  }
}


/*----------------------------------------------------------------*/
void ConvexHull::ConvexHullPoint1::print(void) const
{
  printf("xy=(%d,%d)\n", _x, _y);
}
  
