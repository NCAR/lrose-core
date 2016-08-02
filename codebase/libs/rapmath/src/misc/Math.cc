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
#include <rapmath/Math.hh>
#include <rapmath/usort.h>
using std::vector;
using std::pair;

/*----------------------------------------------------------------*/
// adjust a1 to be as close to a2 as possible, constrained
// to be either a1 or a1 + 180.
double Math::adjustOrientation(double a1, double a2)
{
  double b, d;

  // rotate by subtracting d from both angles, so smaller angle
  // goes to 0.0 and larger angle goes to b
  if (a1 < a2)
  {
    d = a1;
    b = a2 - d;
  }
  else
  {
    d = a2;
    b = a1 - d;
  }

  if (b >= 270.0 || b <= 90.0)
    // b within 90 degrees of 0.0...good as is.
    return a1;
  else
    // take opposite of a1.
    return oppositeAngle(a1);
}

/*----------------------------------------------------------------*/
double Math::vectorLineAngle(double x0, double x1, double y0, double y1,
			     bool is_vert, double slope)
{
  if (is_vert == 1)
  {
    if (y0 < y1)
      return 90.0;
    else
      return 270.0;
  }
  else if (slope == 0.0)
  {
    if (x0 < x1)
      return 0.0;
    else
      return 180.0;
  }
  else
  {
    double x, y, theta;

    y = y1 - y0;
    x = x1 - x0;
    theta = atan2(y, x)*180.0/3.14159;
    while (theta < 0.0)
      theta += 360.0;
    while (theta >= 360.0)
      theta -= 360.0;
    return theta;
  }
}
  
/*----------------------------------------------------------------*/
// return math angle [0,360] from x0,y0 to x1,y1
double Math::vectorAngle(double x0, double y0, double x1, double y1)
{
  double x, y, theta;

  if (veryClose(x0, x1))
  {
    // vertical
    if (y0 < y1)
      return 90.0;
    else
      return 270.0;
  }

  if (veryClose(y0, y1))
  {
    // horizontal
    if (x0 < x1)
      return 0.0;
    else
      return 180.0;
  }

  y = y1 - y0;
  x = x1 - x0;
  theta = atan2(y, x)*180.0/3.14159;
  while (theta < 0.0)
    theta += 360.0;
  while (theta >= 360.0)
    theta -= 360.0;
  return theta;
}

/*----------------------------------------------------------------*/
void Math::sort( const vector<double> &array, vector<int> &indx)
{
  int n = array.size();
  indx.clear();

  double *d = new double[n];
  int *ind = new int[n];
  for (int i=0; i<n; ++i)
  {
    d[i] = array[i];
    ind[i] = i;
  }
  usort_index(d, n, ind);
  for (int i=0; i<n; ++i)
  {
    indx.push_back(ind[i]);
  }
  delete [] d;
  delete [] ind;
}

/*----------------------------------------------------------------*/
void Math::sort( vector<pair<double,int> > &array)
{
  int n = array.size();

  double *d = new double[n];
  int *ind = new int[n];
  for (int i=0; i<n; ++i)
  {
    d[i] = array[i].first;
    ind[i] = array[i].second;
  }
  usort_index(d, n, ind);
  for (int i=0; i<n; ++i)
  {
    array[i].second = ind[i];
  }
  delete [] d;
  delete [] ind;
}

/*----------------------------------------------------------------*/
void Math::sort( int n, double *array, int *indx)
{
  usort_index(array, n, indx);
}

/*----------------------------------------------------------------*/
int Math::maxIndex(vector<double> &v)
{
  int k, maxk;
  double maxv;
  vector<double>::iterator it;

  if (v.size() < 1)
    return -1;

  maxk = 0;
  maxv = *v.begin();

  for (it=v.begin()+1, k=1; it!= v.end(); ++k, ++it)
  {
    if (*it > maxv)
    {
      maxv = *it;
      maxk = k;
    }
  }
  return maxk;
}

/*----------------------------------------------------------------*/
/*
 * Return smallest angle (degrees) from one vector direction 
 * ((vx0,vy0) tail to (vx1,vy1) head) to another direction
 * ((vx1,vy1) tail to (vx2,vy2) head)
 */
double Math::absoluteVectorAngle(double vx0, double vy0,
				   double vx1, double vy1,
				   double vx2, double vy2)
{
  double dx, dy, r, dx2, dy2, h, v, a;

  // let dx,dy = unit vector in direction from vx0,vy0 to vx1,vy1
  dx = vx1 - vx0;
  dy = vy1 - vy0;
  r = sqrt(dx*dx + dy*dy);
  dx = dx/r;
  dy = dy/r;

  // now take dot product of dx,dy and vector from vx1,vy1 to
  // vx2, vy2
  dx2 = vx2 - vx1;
  dy2 = vy2 - vy1;
  h = dx2*dx + dy2*dy;

  // this is the 'horizontal' (relative to dx,dy) component of
  // dx2, dy2. The 'vertical' component is gotten by figuring
  // total length.
  v = sqrt(dx2*dx2 + dy2*dy2 - h*h);
    
  // h and v are used to figure the angle.
  a = fabs(atan2(v, h)*180.0/3.14159);

  // if h is non-negative, the angle doesn't exceed 90.
  // otherwise it must.
  if (h >= 0.0)
  {
    if (a > 90.0)
      a = 180.0 - a;
  }
  else
  {
    if (a < 90.0)
      a = 180.0 - a;
  }
  return a;
}

/*----------------------------------------------------------------*/
void Math::rotatePoint(double &x, double &y, double angle)
{
  double a, cosa, sina, x0, y0;
    
  a = angle*3.14159/180.0;
  cosa = cos(a);
  sina = sin(a);
  x0 = x*cosa  + y*sina;
  y0 = -x*sina + y*cosa;
  x = x0;
  y = y0;
}

/*----------------------------------------------------------------*/
bool Math::anglesTooFarApart(double aprev, double a, double max_change)
{
  if (aprev < max_change)
    return (a > (aprev+max_change) && (a < (360.0 - max_change + aprev)));
  else if (aprev == max_change)
    return (a > aprev + max_change);
  else if (aprev > max_change && aprev < 360.0 - max_change)
    return (a < aprev-max_change || a > aprev+max_change);
  else if (aprev == 360.0 - max_change)
    return (a < aprev - max_change);
  else
    return (a < (aprev - max_change) &&
	    (a > (max_change - 360.0 + aprev)));
}

