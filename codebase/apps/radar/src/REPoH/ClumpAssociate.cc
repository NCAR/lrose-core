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
 * @file ClumpAssociate.cc
 * @brief ClumpAssociate main class
 */
#include "ClumpAssociate.hh"
#include "ClumpAssociate1.hh"
#include "ClumpRegions.hh"
#include "CloudGap.hh"
#include "Sweep.hh"
#include <algorithm>
#include <cmath>

using std::vector;

/*----------------------------------------------------------------*/
static bool _inside_pid_clump(const int x, const int y, 
			      const Grid2d &pid_clumps,
			      const ClumpAssociate1 &ca, 
			      vector<int> &pid_colors)
{
  double v;
  if (!pid_clumps.getValue(x, y, v))
    return false;
  int iv = (int)v;
  for (int i=0; i<ca.numColor(); ++i)
  {
    int c = ca.ithColor(i);
    if (c == iv)
    {
      if (find(pid_colors.begin(), pid_colors.end(), c) == pid_colors.end())
	pid_colors.push_back(c);
      return true;
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
static int _compute_penetration(const ClumpAssociate1 &ca, int x0, int y, 
				const Grid2d &pid_clumps, const bool down,
				vector<int> &pid_colors)
{
  int ncolor = ca.numColor();
  if (ncolor == 0)
    return 0;

  int npt = 0;
  if (down)
  {
    // keep backing down while in a pid clump associated with ca
    for (int x=x0-1; x>=0; --x)
    {
      if (!_inside_pid_clump(x, y, pid_clumps, ca, pid_colors))
	  break;
      ++npt;
    }
  }
  else
  {
    // keep moving forward while in a pid clump associated with ca
    for (int x=x0+1; x<pid_clumps.getNx(); ++x)
    {
      if (!_inside_pid_clump(x, y, pid_clumps, ca, pid_colors))
	  break;
      ++npt;
    }
  }
  return npt;
}

/*----------------------------------------------------------------*/
ClumpAssociate::ClumpAssociate(const ClumpRegions &r,
			       const Grid2d &clumps,
			       const Grid2d &biggerClumps) : MathUserData()
{
  for (int i=0; i<(int)r.size(); ++i)
  {
    double color = Sweep::indexToColor(i);
    ClumpAssociate1 c1((int)color);

    for (int j=0; j<(int)r[i].size(); ++j)
    {
      int x = (int)(r[i].ithX(j));
      int y = (int)(r[i].ithY(j));
      double v;
      if (biggerClumps.getValue(x, y, v))
      {
	c1.addColor((int)v);
      }
    }
    _elem.push_back(c1);
  }
}

/*----------------------------------------------------------------*/
ClumpAssociate::~ClumpAssociate()
{
}

/*----------------------------------------------------------------*/
bool ClumpAssociate::getFloat(double &f) const
{
  return false;
}

/*----------------------------------------------------------------*/
void ClumpAssociate::print(void) const
{
  for (int i=0; i<(int)_elem.size(); ++i)
    _elem[i].print();
}

/*----------------------------------------------------------------*/
void ClumpAssociate::penetration(const CloudGap &gap, const Grid2d &pidClumps,
				 int &npt0, int &npt1)
{
  bool is_far = true;

  _x0 = gap.getX(is_far);
  _x1 = gap.getX(!is_far);
  _y = gap.getY();
  _color0 = (int) floor(gap.getValue(is_far) + 0.5);
  _color1 = (int) floor(gap.getValue(!is_far) + 0.5);
  _pid_colors0.clear();
  _pid_colors1.clear();

  // test 0th end, find the color
  const ClumpAssociate1 *c0 = _get_matching(_color0);
  if (c0 == NULL)
  {
    npt0 = 0;
  }
  else
  {
    npt0 = _compute_penetration(*c0, _x0, _y, pidClumps, true, _pid_colors0);
  }
  
  // test 1th end
  c0 = _get_matching(_color1);
  if (c0 == NULL)
    npt1 = 0;
  else
    npt1 = _compute_penetration(*c0, _x1, _y, pidClumps, false, _pid_colors1);
}

/*----------------------------------------------------------------*/
vector<string> ClumpAssociate::get_penetration_report(void) const
{
  vector<string> ret;
  string s;
  char buf[1000];
  sprintf(buf, "[%d,%d] color=%d  pid_colors=( ", _x0, _y, _color0);
  s = buf;
  for (int i=0; i<(int)_pid_colors0.size(); ++i)
  {
    sprintf(buf, "%d ", _pid_colors0[i]);
    s += buf;
  }
  s += ")";
  ret.push_back(s);

  sprintf(buf, "[%d,%d] color=%d  pid_colors=( ", _x1, _y, _color1);
  s = buf;
  for (int i=0; i<(int)_pid_colors1.size(); ++i)
  {
    sprintf(buf, "%d ", _pid_colors1[i]);
    s += buf;
  }
  s += ")";
  ret.push_back(s);
  return ret;
}


/*----------------------------------------------------------------*/
const ClumpAssociate1 *ClumpAssociate::_get_matching(const int color) const
{
  for (int i=0; i<(int)_elem.size(); ++i)
    if (_elem[i].getColor() == color)
    {
      return &_elem[i];
    }
  return NULL;
}

