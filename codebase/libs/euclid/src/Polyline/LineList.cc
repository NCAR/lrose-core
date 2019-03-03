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
#include <euclid/LineList.hh>
#include <euclid/Grid2d.hh>
#include <euclid/MotionVector.hh>
#include <euclid/Point.hh>
#include <euclid/Box.hh>

#include <rapmath/AngleCombiner.hh>
#include <rapmath/OrderedList.hh>
#include <rapmath/Math.hh>
#include <rapmath/FuzzyF.hh>


#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>
#include <algorithm>
#include <cstdio>
using std::string;
using std::vector;
using std::pair;

/*----------------------------------------------------------------*/
static double _averageSpeeds1(vector<double> &speed, 
			      vector<double> &len,
			      int index, int num,
			      double speed_smoothing_len)
{
  int i, min, max;
  double s, n, l;

  /*
   * get sum of lines prior to index whose combined length just exceeds param
   */
  l = 0.0;
  min = 0;
  for (i=index-1; i>=0; --i)
  {
    l += *(len.begin() + i);
    min = i;
    if (l >= speed_smoothing_len)
      break;
  }

  /*
   * get sum of lines beyond index whose combined length just exceeds param
   */
  l = 0.0;
  max = num - 1;
  for (i=index+1; i<num; ++i)
  {
    l += *(len.begin() + i);
    max = i;
    if (l >= speed_smoothing_len)
      break;
  }

  /*
   * Take average of speeds within these bounds
   */
  s = n = 0.0;
  for (i=min; i<=max; ++i)
  {
    s += *(speed.begin() +i);
    ++n;
  }
  if (n > 0)
    return s/n;
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
// reconnect, so that 1th endpoint of last thing in lnew
// connects with 0th of input line, which has also been added on exit.
static void _reconnectAsNeeded(const Line &l1, LineList &lnew)
{
  int i;
  double x0, y0, x1, y1;
    
  i = lnew.num();
  Line *l0 = lnew.ithLinePtr(i-1);
  Line l2(l1);

  // do the endpoints agree within a very small tolerance?
  l0->point(1, x0, y0);
  l1.point(0, x1, y1);
  if (Math::veryClose(x0, x1) &&
      Math::veryClose(y0, y1))
    l2.adjustEndpoint(0, x0, y0);
  else
  {
    double x0, y0;
      
    // do the lines intersect?
    if (!l0->intersect(l2, x0, y0))
    {
      // get average point
      l0->point(1, x0, y0);
      l2.point(0, x1, y1);
      x0 = (x0 + x1)/2.0;
      y0 = (y0 + y1)/2.0;
    }
    l0->adjustEndpoint(1, x0, y0);
    l2.adjustEndpoint(0, x0, y0);
  }
  lnew.append(l2);
}

/*----------------------------------------------------------------*/
// given a line and a list, build the lines from list that are
// close to line.
static LineList _buildNearbyList(const Line &line, const LineList &list, 
				 double maxdist)
{
  LineList ret;
  int i;

  // build all lines within some distance of input line from list.
  for (i=0; i<list.num(); ++i)
  {
    Line li = list.ithLine(i);
    if (line.minimumDistanceBetween(li) < maxdist)
    {
      ret.append(li);
    }
  }
  return ret;
}
/*----------------------------------------------------------------*/
static void _hv0(double x0, double y0, double a0, double x1,
		 double y1, double &h, double &v)
{
  double r, a, xr, yr, xr0, xr1, yr0, yr1;

  // make a line forward from x0,y0
  // make it horizontal, with point 1 pointing 'forward'.
  r = a0*3.14159/180.0;
  Line l0(x0, y0, x0 + cos(r), y0 + sin(r));
  l0.makeHorizontal(a, false);
  l0.point(0, xr0, yr0);
  l0.point(1, xr1, yr1);

  // rotate the other point by this angle.
  Point xy(x1, y1);
  xy.rotate(a);
  xr = xy.getX();
  yr = xy.getY();

  if (xr1 >= xr0)
    h = xr - xr0;
  else
    h = xr0 - xr;
  if (h < 0.0)
    h = 0.0;
  v = fabs(yr - yr0);
}

/*----------------------------------------------------------------*/
static void _horiz_vert(double x0, double y0, double a0,
			double x1, double y1, double a1,
			double &h0, double &v0, double &h1, double &v1)
{
  _hv0(x0, y0, a0, x1, y1, h0, v0);
  _hv0(x1, y1, a1, x0, y0, h1, v1);
}

/*----------------------------------------------------------------*/
// return a measure of where a list is based on these inputs:
// line = line from list, assumed closest to where we want to be.
// l = list.
// a = average angle of list.
static void _location(const Line &line, const LineList &l, double a, double &x,
		      double &y, double &ahead)
{
  LineList lnew(l);
  Line line_new(line);
  double x0, x1;
  double lx0, lx1, ly, d00, d01, d10, d11;
    
  // make the list and line approximately 'horizontal'.
  lnew.rotate(a, false);
  line_new.rotate(a, false);

  // get the rotated endpoints of line_new, one of which is our extreme
  // point in x.
  line_new.point(0, lx0, ly);
  line_new.point(1, lx1, ly);

  // get extrema of the whole box.
  Box b = lnew.extrema();

  b.getRange(x0, x1, true);

  // now one end of the line should be close to the box boundary in x.
  // figure that out and then back away some percentage...
  d00 = fabs(x0 - lx0);
  d01 = fabs(x0 - lx1);
  d10 = fabs(x1 - lx0);
  d11 = fabs(x1 - lx1);

  Point axy;
  if (d00 <= d01 && d00 <= d10 && d00 <= d11)
  {
    x = lx0 + 0.1*(x1 - lx0);
    axy = Point(-1.0, 0.0);
  }
  else if (d01 <= d00 && d01 <= d10 && d01 <= d11)
  {
    x = lx1 + 0.1*(x1 - lx1);
    axy = Point(-1.0, 0.0);
  }
  else if (d10 <= d00 && d10 <= d01 && d10 <= d11)
  {
    x = lx0 - 0.1*(lx0 - x0);
    axy = Point(1.0, 0.0);
  }
  else
  {
    x = lx1 - 0.1*(lx1 - x0);
    axy = Point(1.0, 0.0);
  }

  // y is set to middle of box.
  y = b.average(false);
    
  // rotate back..
  Point xy(x, y);
  xy.rotate(-a);
  x = xy.getX();
  y = xy.getY();
  axy.rotate(-a);
  ahead = atan2(axy.getY(), axy.getX())*180.0/3.14159;
}

/*----------------------------------------------------------------*/
// line0 is line from list0 nearest list1.
// line1 is line from list1 nearest list0
// angle_out_0 is angle from list0 towards list1
// angle_out_1 is angle from list1 towards list0.
static bool _proximate(const Line &line0, const LineList &list0,
		       const Line &line1, const LineList &list1,
		       double angle_out_0, double angle_out_1,
		       const FuzzyF &f)
{
  double a0, a1, x0, y0, x1, y1, h0, v0, h1, v1, d, maxd, b0, b1;

  // make sure the linelists point 'toward' each other..the angles
  // out should be approximately opposite...if they are instead
  // approximately equal, that is bad and give up..
  d = Math::angleDiff(angle_out_0, angle_out_1);
  if (d < 40.0)
    return false;
    
  // build up a representative orientation for each  of the two lists.
  // this is the length weighted fuzzy average.
  if (!list0.averageOrientation(a0) ||
      !list1.averageOrientation(a1))
    return false;

  // build up a representative location for each of the two lists.
  // its kind of an average point.
  _location(line0, list0, a0, x0, y0, b0);
  _location(line1, list1, a1, x1, y1, b1);

  // get orientation diff.
  d = Math::angleDiff(a0, a1);

  // get horizontal and vertical distances between the two, going both ways.
  _horiz_vert(x0, y0, b0, x1, y1, b1, h0, v0, h1, v1);

  // use those to decide whether proximate or not..
  maxd = f.apply(d);

  // are the lines close together horizontally and vertically?
  if (h0 > maxd || h1 > maxd)
    return false;
  if (v0 > maxd || v1 > maxd)
    return false;
  return true;
}

/*----------------------------------------------------------------*/
// first double is orientation, second is confidence or weight..
static void _bigSmallOrientationAdjust(vector<pair<double,double> > &o)
{
  bool large, small;
  vector<pair<double,double> >::iterator oit;
  double d;

  large = small = false;
  for (oit=o.begin(); oit != o.end(); ++oit)
  {
    d = oit->first; // data value
    if (d > 135.0)
      large = true;
    if (d < 45.0)
      small = true;
  }
  if (large && small)
  {
    // subtract 180 from all values > 90.
    for (oit=o.begin(); oit != o.end(); ++oit)
    {
      if (oit->first > 90.0)
	oit->first -= 180.0;
    }
  }
}

/*----------------------------------------------------------------*/
// first in pair is data, second is weight.
static double _anglesConf(vector<pair<double, double> > &o)
{
  // get range and average spread from median
  vector<pair<double,double> >::iterator it;
  double min=0.0, max=0.0, m, p1, v;
  // double p0 = 0.0;

  OrderedList ol;
  for (it=o.begin(); it!=o.end(); ++it)
  {
    v = it->first;
    ol.addToListUnordered(v);
    if (it == o.begin())
      min = max = v;
    else
    {
      if (v < min)
	min = v;
      if (v > max)
	max = v;
    }
  }
  // p0 = (max - min);

  ol.order();

  // hardwired stuff here!
  max = ol.percentile(0.85);
  min = ol.percentile(0.15);
    
  p1 = (max - min);

  if (p1 < 20.0)
    v = 1.0;
  else if (p1 >= 160.0)
    v =  0.0;
  else
  {
    m = -1.0/140.0;
    v = (m*(p1 -20.0) + 1.0);
  }
  if (v >= 0.95)
    v = 0.95;
  return v;
}

/*----------------------------------------------------------------*/
LineList::LineList() : AttributesEuclid()
{
  _init();
}

/*----------------------------------------------------------------*/
LineList::LineList(const Line &line, double max_length) : AttributesEuclid()
{
  _init();
  double alpha0, alpha1, x0, x1, y0, y1, len;

  len = line.length();

  // number of lines needed to not exceed max_length:
  int num_lines = (int)((len- Math::smallValue())/max_length) + 1;
  if (num_lines == 1)
  {
    append(line);
    return;
  }

  // delta = length of each line when interpolating.
  double delta = len/(double)num_lines;

  // this copies in all attributes, including motion.
  Line ltemplate = line;

  for (int i=num_lines-1; i>= 0.0; --i)
  {
    // get parametric endpoint locations.
    alpha0 = (double)(i+1)*delta/len;
    alpha1 = (double)(i)*delta/len;

    // figure out the two endpoints.
    x0 = line.xAtParametric(alpha0);
    y0 = line.yAtParametric(alpha0);
    x1 = line.xAtParametric(alpha1);
    y1 = line.yAtParametric(alpha1);

    // store to template line and append.
    ltemplate.adjustEndpoints(x0, y0, x1, y1);
    append(ltemplate);
  }
}

/*----------------------------------------------------------------*/
LineList::~LineList()
{
}

/*----------------------------------------------------------------*/
LineList::LineList(const LineList &l) : AttributesEuclid(l)
{
  _line = l._line;
}

/*----------------------------------------------------------------*/
LineList & LineList::operator=(const LineList &l)
{
  if (&l == this)
  {
    return *this;
  }

  _line = l._line;
  Attributes::operator=(l);
  return *this;
}

/*----------------------------------------------------------------*/
bool LineList::operator==(const LineList &l) const
{
  return _line == l._line && Attributes::operator==(l);
}

/*----------------------------------------------------------------*/
string LineList::writeXml(const std::string &tag) const
{
  string ret = TaXml::writeStartTag(tag, 0);
  ret += Attributes::writeAttXml("LineListAttributes");
  for (size_t i=0; i<_line.size(); ++i)
  {
    ret += _line[i].writeXml("Line");
  }
  ret += TaXml::writeEndTag(tag, 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool LineList::readXml(const std::string &xml, const std::string &tag)
{
  string buf;
  if (TaXml::readString(xml, tag, buf))
  {
    LOG(ERROR) << "Finding key " << tag << " in buffer";
    return false;
  }
  if (!Attributes::readAttXml(buf, "LineListAttributes"))
  {
    return false;
  }
  vector<string> lines;
  if (TaXml::readStringArray(buf, "Line", lines))
  {
    LOG(DEBUG) << "No Line array in linelist, assume no lines";
  }
  else
  {
    for (size_t i=0; i<lines.size(); ++i)
    {
      Line l;
      if (l.readXml(lines[i]))
      {
	_line.push_back(l);
      }
      else
      {
	return false;
      }
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
void LineList::print(FILE *fp) const
{
  fprintf(fp, "linelist nline:%d ", (int)_line.size());
  printAtt(fp);
  printf("\n");
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].print(fp);
}

/*----------------------------------------------------------------*/
void LineList::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
// add things from l not yet there.
void LineList::formUnion(const LineList &l)  // add new things.
{
  Attributes::attributeUnion(l);
  for (int i=0; i<(int)l._line.size(); ++i)
  {
    if (find(_line.begin(), _line.end(), l._line[i]) == _line.end())
      append(l._line[i]);
  }
}
  
/*----------------------------------------------------------------*/
bool LineList::closestDistanceSquared(const double x0, const double y0,
				      double &dist, int &index) const
{
  bool first = true;
    
  for (int i=0; i<(int)_line.size(); ++i)
  {
    double di = _line[i].minDistanceSquared(x0, y0);
    if (first)
    {
      first = false;
      dist = di;
      index = i;
    }
    else
    {
      if (di < dist)
      {
	dist = di;
	index = i;
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool LineList::dataWeightedOrientation(int x, int y, 
				       const string &attribute_name,
				       int radius,
				       double percentile0,
				       double percentile1,
				       double &orientation,
				       double &conf) const
{
  vector<pair<double,double> > o; // first is data, second weight.
  vector<pair<double,double> >::iterator it; 
    
  // build up orientations/weights vectors from line attributes.
  o = _createDataweightVector(x, y, attribute_name, radius);
  if (o.size() == 0)
    return false;
    
  // now check for big/large values and adjust accordingly.
  _bigSmallOrientationAdjust(o);

  // now create an order list in terms of weights w.
  OrderedList ol;
  for (it=o.begin(); it != o.end(); ++it)
    ol.addToListUnordered(*it);
  ol.orderWeights();

  // take average between p0 and p1 values (weights).
  orientation = ol.weightConstrainedAverage(percentile0, percentile1);

  // is it too noisy to use?
  conf = _anglesConf(o);
  if (orientation < 0.0)
    orientation += 180.0;
  return true;
}


/*----------------------------------------------------------------*/
void LineList::addIndividualAttributeDouble(const std::string &name, double v)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].addDouble(name, v);
  }
}

/*----------------------------------------------------------------*/
void LineList::removeIndividualAttributeDouble(const std::string &name)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].removeDouble(name);
  }
}

/*----------------------------------------------------------------*/
void LineList::addIndividualAttributeInt(const std::string &name, int v)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].addInt(name, v);
  }
}

/*----------------------------------------------------------------*/
void LineList::removeIndividualAttributeInt(const std::string &name)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].removeInt(name);
  }
}

/*----------------------------------------------------------------*/
void LineList::addIndividualAttributeTime(const time_t &t)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].setTime(t);
  }
}

/*----------------------------------------------------------------*/
void LineList::addIndividualAttributeMotionVector(const MotionVector &mv)
{
  for (size_t i=0; i<_line.size(); ++i)
  {
    _line[i].setMotionVector(mv);
  }
}

/*----------------------------------------------------------------*/
LineList LineList::connectOpposing(const LineList &l) const
{
  // special cases where one or both lists are empty.
  if (l.num() == 0)
    return *this;
  else
  {
    if (num() == 0)
      return l;
    else
      return _connectOpposingNonTrivial(l);
  }
}

/*----------------------------------------------------------------*/
void LineList::appendToList(const LineList &l)
{
  for (int i=0; i<(int)l._line.size(); ++i)
    _line.push_back(l._line[i]);
}

/*----------------------------------------------------------------*/
void LineList::reverseOrder(void)
{
  LineList tmp;
  int n = (int)_line.size();
  for (int i=n-1; i>=0; --i)
  {
    Line l = _line[i];
    l.reverse();
    tmp.append(l);
  }
  *this = tmp;
}

/*----------------------------------------------------------------*/
void LineList::clearBetween(Grid2d &tmp) const
{
  Line l0, ln;
  
  if (num() < 1)
    return;
    
  l0 = ithLine(0);
  ln = ithLine(num()-1);
  ln.clearBetween(l0, tmp);
}

/*----------------------------------------------------------------*/
double LineList::length(void) const
{
  double ret = 0;
  for (int i=0; i<(int)_line.size(); ++i)
    ret += _line[i].length();
  return ret;
}

/*----------------------------------------------------------------*/
double LineList::cumulativeLength(void) const
{
  if (_line.size() == 0)
    return 0.0;

  double len = 0.0;
  if (isConnected())
  {
    for (int i=0; i<(int)_line.size(); ++i)
      len += _line[i].length();
  }
  else
  {
    if ((int)_line.size() == 1)
      len = _line[0].length();
    else
    {
      for (int i=1; i<(int)_line.size(); ++i)
      {
	double x0, y0, x1, y1, x, y;
	_line[i-1].centerpoint(x0, y0);
	_line[i].centerpoint(x1, y1);
	x = x1-x0;
	y = y1-y0;
	len += sqrt(x*x + y*y);
      }
    }
  }
  return len;
}

/*----------------------------------------------------------------*/
double LineList::nonsubsetLength(int i0, int i1) const
{
  double len;
  int i;

  len = 0.0;
  for (i=0; i<i0; ++i)
    len += ithLine(i).length();
  for (i= i1+1; i<num()-1; ++i)
    len += ithLine(i).length();
  return len;
}

/*----------------------------------------------------------------*/
void LineList::removeFromEnd(int which, double len)
{
  if (which == 0)
    _removeUp(len);
  else
    _removeDown(len);
}

/*----------------------------------------------------------------*/
bool LineList::removeElement(const Line &a)
{
  vector<Line>::iterator it;
  
  for (it=_line.begin(); it != _line.end(); ++it)
  {
    if (*it == a)
    {
      _line.erase(it);
      return true;
    }
  }
  return false;
}
  
/*----------------------------------------------------------------*/
bool LineList::removeElement(int i)
{
  if (i < 0 || i >= (int)_line.size())
    return false;
  _line.erase(_line.begin()+i);
  return true;
}
  
/*----------------------------------------------------------------*/
bool LineList::removeElements(int first_to_remove, int last_to_remove)
{
  vector<Line>::iterator it;
  int i;

  if (first_to_remove < 0 || last_to_remove >= (int)_line.size())
    return false;
  if (first_to_remove > last_to_remove)
    return false;
  for (i=0,it=_line.begin(); it != _line.end(); ++i)
  {
    if (i >= first_to_remove && i <= last_to_remove)
    {
      it = _line.erase(it);
    }
    else
      ++it;
  }
  return true;
}

/*----------------------------------------------------------------*/
void LineList::addMotion(const Grid2d &motion_angle,
			  const Grid2d &motion_magnitude, bool debug)
{
  int i;

  // change motion info for each line in ml.
  Grid2d mask(motion_angle);
  mask.setAllMissing();
  for (i=0; i<num(); ++i)
  {
    ithLinePtr(i)->addMotion(mask, motion_angle, 
			     motion_magnitude, debug);
  }
}

/*----------------------------------------------------------------*/
void LineList::spacingFilter(double max_length)
{
  LineList tmp;

  // for each line:
  for (int i=0; i<num(); ++i)
  {
    Line li = ithLine(i);

    // break it into more lines into a linelist using special constructor.
    LineList lli(li, max_length);

    // save those lines into new list.
    for (int j=0; j<lli.num(); ++j)
      tmp.append(lli.ithLine(j));
  }
  *this = tmp;
}

/*----------------------------------------------------------------*/
string LineList::sprintEndsAndNum(const string &name) const
{
  string ret;
  double x0, y0, x1, y1;

  if (!_ends(x0, y0, x1, y1))
    return ret;
  char buf[100];
  sprintf(buf, "%s: [(%.2f,%.2f)  (%.2f,%.2f)]  nline:%d",
	  name.c_str(), x0, y0, x1, y1, num());
  ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
string LineList::sprintEndsAndNum(const string &name, int index) const
{
  string ret;
  double x0, y0, x1, y1;

  if (!_ends(x0, y0, x1, y1))
    return ret;
  char buf[100];
  sprintf(buf, "%s[%d]: [(%.2f,%.2f)  (%.2f,%.2f)]  nline:%d",
	  name.c_str(), index, x0, y0, x1, y1, num());
  ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
void LineList::appendLines(const LineList &l, int i0,  int i1,
			    bool reverse)
{
  int inc, i;
    
  if (i0 <= i1)
    inc = +1;
  else
    inc = -1;
	
  for (i=i0; ; i+= inc)
  {
    Line line = l.ithLine(i);
    if (reverse)
      line.reverse();
    append(line);
    if (i == i1)
      break;
  }
}

/*----------------------------------------------------------------*/
bool LineList::isConnected(void) const
{
  if (_line.size() < 2)
  {
    return true;
  }

  for (size_t i=1; i<_line.size(); ++i)
  {
    if (_line[i]._x0 != _line[i-1]._x1 || _line[i]._y0 != _line[i-1]._y1)
    {
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
void LineList::makeConnected(void)
{
  if (_line.size() < 2)
  {
    return;
  }
  // copy to get attributes
  LineList lnew(*this);

  // clear out lines from this and build back up
  clear();
  _line.push_back(lnew._line[0]);

  for (size_t i=1; i<lnew._line.size(); ++i)
  {
    if (lnew._line[i]._x0 != lnew._line[i-1]._x1 ||
	lnew._line[i]._y0 != lnew._line[i-1]._y1)
    {
      // add a new line that forces connection
      _line.push_back(Line(lnew._line[i-1]._x1, lnew._line[i-1]._y1,
			   lnew._line[i]._x0, lnew._line[i]._y0));
    }
    _line.push_back(lnew._line[i]);
  }
}

/*----------------------------------------------------------------*/
void LineList::makeConnectedAndRemoveSmall(double small)
{
  makeConnected();
  removeSmall(small, true);
}

/*----------------------------------------------------------------*/
bool LineList::centerLocation(double &x, double &y) const
{
  double l, lsum, leni, p, d;
  int i;

  l = length()/2.0;
  for (lsum=0.0,i=0; i<num(); ++i)
  {
    Line li = ithLine(i);
    leni = li.length();
    if (lsum + leni > l)
    {
      // the ith line put us over the top...get percent of
      // ith line to get exactly l.
      d = l - lsum;
      p = d/leni;
	    
      // now get location of this position along li.
      x = li.xAtParametric(1.0 - p);
      y = li.yAtParametric(1.0 - p);
      return true;
    }
    lsum += leni;
  }

  // here never got there...must be a 0 length linelist.
  return false;
}

/*----------------------------------------------------------------*/
bool LineList::averageSeparation(const LineList &l1, double &sep) const
{
  int i, j;
  double dist, d, n;

  dist = 0.0;
  n = 0.0;
  for (i=0; i<num(); ++i)
  {
    Line t = ithLine(i);
    for (j=0; j<l1.num(); ++j)
    {
      Line t1 = l1.ithLine(j);
      if (t.averageLineDistance(t1, d))
      {
	dist += d;
	++n;
      }
    }
  }
  if (n > 0)
  {
    sep = dist/n;
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
double LineList::lengthWeightedSpeed(void) const
{
  double d, s, si, li;
  int i;

  for (d=s=0.0, i=0; i<num(); ++i)
  {
    Line t = ithLine(i);
    if (t.getMotionSpeed(si))
    {
      li = t.length();
      d += li;
      s += si*li;
    }
  }

  if (d != 0.0)
    return s/d;
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
bool LineList::proximateBoundaries(const LineList &l1,
				    double maxdist,
				    const FuzzyF &angle_maxdist) const
{
  double x00, y00, x01, y01, x10, y10, x11, y11, d00, d01, d10, d11;
  double a0, a1;
  LineList list0, list1;
  bool stat;

  if (l1.num() == 0 || num() == 0)
  {
    // empty linelist can't be proximate to anything
    return false;
  }

  if (!isConnected() || !l1.isConnected())
  {
    return false;
  }

  // figure out which end of which boundary is closest to which end
  // of the other one.
  Line line00 = ithLine(0);
  line00.point(0, x00, y00);
  Line line01 = ithLine(num()-1);
  line01.point(1, x01, y01);
  Line line10 = l1.ithLine(0);
  line10.point(0, x10, y10);
  Line line11 = l1.ithLine(l1.num()-1);
  line11.point(1, x11, y11);

  Line l0_near1, l1_near0;

  // see which is closest, 00 to 10, 00 to 11, 01 to 10 01 to 11
  d00 = (x00 - x10)*(x00 - x10) + (y00 - y10)*(y00-y10);
  d01 = (x00 - x11)*(x00 - x11) + (y00 - y11)*(y00-y11);
  d10 = (x01 - x10)*(x01 - x10) + (y01 - y10)*(y01-y10);
  d11 = (x01 - x11)*(x01 - x11) + (y01 - y11)*(y01-y11);

  // a0 = angle from linelist 0 towards linelist 1
  // a1 = angle from linelist 1 towards linelist 0
  // l0_near1 = line from list0 near list1
  // l1_near0 = line from list1 near list0
  if (d00 <= d01 && d00 <= d10 && d00 <= d11)
  {
    // order of endpoints: 01,00,10,11
    a0 = line00.vectorAngleFromEnd(1);
    a1 = line10.vectorAngleFromEnd(1);
    l0_near1 = line00;
    l1_near0 = line10;
  }
  else if (d01 <= d00 && d01 <= d10 && d01 <= d11)
  {
    // order of endpoints: 01,00,11,10
    a0 = line00.vectorAngleFromEnd(1);
    a1 = line11.vectorAngleFromEnd(0);
    l0_near1 = line00;
    l1_near0 = line11;
  }
  else if (d10 <= d00 && d10 <= d01 && d10 <= d11)
  {
    // order of endpoints: 00,01,10,11
    a0 = line01.vectorAngleFromEnd(0);
    a1 = line10.vectorAngleFromEnd(1);
    l0_near1 = line01;
    l1_near0 = line10;
  }
  else // d11 is minimal.
  {
    // order of endpoints: 00,01,11,10
    a0 = line01.vectorAngleFromEnd(0);
    a1 = line11.vectorAngleFromEnd(0);
    l0_near1 = line01;
    l1_near0 = line11;
  }        

  // list0 = lines from linelist 0 near linelist 1
  // list1 = lines from linelist 1 near linelist 0
  list0 = _buildNearbyList(l0_near1, *this, maxdist);
  list1 = _buildNearbyList(l1_near0, l1, maxdist);
  stat = _proximate(l0_near1, list0, l1_near0, list1, a0, a1,
		    angle_maxdist);
  return stat;
}

/*----------------------------------------------------------------*/
void LineList::rotate(double angle, bool change_endpts)
{
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].rotate(angle, change_endpts);
}

/*----------------------------------------------------------------*/
Box LineList::extrema(void) const
{
  Box b, big_b;
  bool first = true;
    
  for (int i=0; i<(int)_line.size(); ++i)
  {
    b = _line[i].extrema();
    if (b.emptyBox())
      continue;
    if (first)
    {
      big_b = b;
      first = false;
    }
    else
      big_b.expand(b);
  }
  return big_b;
}

/*----------------------------------------------------------------*/
bool LineList::averageOrientation(double &a) const
{
  AngleCombiner f(num());
  int i;

  for (i=0; i<num(); ++i)
  {
    a = _line[i].vectorAngleFromEnd(0);
    if (a >= 180.0)
      a = a - 180.0;
    f.setGood(i, a, _line[i].length());
  }
  return f.getCombineAngle(a);
}

/*----------------------------------------------------------------*/
bool LineList::averageOrientation(double p, int which, double &a) const
{
  AngleCombiner f(num(), true);
  int i, i0, i1, di, vi, j;
  double len, l;
  bool ok;

  len = length();
  len = len*p;
  if (which == 0)
  {
    i0 = 0;
    di = 1;
    i1 = num();
    vi = 1;
  }
  else
  {
    i0 = num()-1;
    di = -1;
    i1 = -1;
    vi = 0;
  }

  l = 0.0;
  for (ok=false,j=0,i=i0; i!=i1; i += di,++j)
  {
    Line li = ithLine(i);
    l += li.length();
    if (l > len && ok)
      break;
    a = li.vectorAngleFromEnd(vi);
    f.setGood(j, a, li.length());
    ok = true;
    if (l > len)
      break;
  }
  return f.getCombineAngle(a);
}

/*----------------------------------------------------------------*/
double LineList::maxSpeed(void) const
{
  double max=0.0, s;
  bool first=true;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (_line[i].getMotionSpeed(s))
    {
      if (first)
      {
	first = false;
	max = s;
      }
      else
      {
	if (s > max)
	  max = s;
      }
    }
  }
  if (first)
    return 0.0;
  else
    return max;
}

/*----------------------------------------------------------------*/
bool LineList::getAveQuality(double &a) const
{
  a=0.0;
  double a0, num=0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (_line[i].getQuality(a0))
    {
      a += a0;
      ++num;
    }
  }
  if (num > 0)
  {
    a = a/num;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool LineList::getMaxQuality(double &m) const
{
  bool first = true;
  double a0;

  m=0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (_line[i].getQuality(a0))
    {
      if (first)
      {
	first = false;
	m = a0;
      }
      else
      {
	if (a0 > m)
	  m = a0;
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool LineList::vectorIntersects(double x, double y, double deg, double &xp,
				 double &yp) const
{
  int i;
  for (i=0; i<num(); ++i)
  {
    Line t = ithLine(i);
    if (t.vectorIntersects(x, y, deg, xp, yp))
      return true;
  }
  xp = yp = 0.0;
  return false;
}

/*----------------------------------------------------------------*/
void LineList::extend(int which, double plen, double pdir)
{
  double a, x0, y0, x1, y1, l;

  if (num() <= 0)
    return;
  if (!averageOrientation(pdir, which, a))
    return;
  a = a*3.14159/180.0;
  l = plen*length();
  if (which == 0)
    ithLine(0).point(0, x0, y0);
  else
    ithLine(num()-1).point(1, x0, y0);
  x1 = x0 + l*cos(a);
  y1 = y0 + l*sin(a);
  Line lnew(x0, y0, x1, y1);
  if (which == 0)
  {
    // prepend this new line, reversed..
    lnew.reverse();
    _line.insert(_line.begin(), lnew);
  }        
  else
    // append this new line, as is.
    _line.push_back(lnew);
}

/*----------------------------------------------------------------*/
bool LineList::velocitiesAllZero(void) const
{
  for (int i=0; i<(int)_line.size(); ++i)
  {
    double s;
    if (_line[i].getMotionSpeed(s))
    {
      if (s != 0.0)
	return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
void LineList::reconnectAsNeeded(void)
{
  if (num() <= 1)
    return;

  LineList lold(*this);
  int i;

  clear();

  // add 0th to list as is.
  Line l = lold.ithLine(0);
  append(l);
  for (i=1; i<lold.num(); ++i)
  {
    // merge 1th into current list
    l = lold.ithLine(i);
    _reconnectAsNeeded(l, *this);
  }
  adjustMotionDirections();
}

/*----------------------------------------------------------------*/
void LineList::adjustMotionDirections(void)
{
  int i;
  Line *t;
    
  for (i=0; i<num(); ++i)
  {
    t = ithLinePtr(i);
    t->adjustMotionDirection();
  }
}

/*----------------------------------------------------------------*/
void LineList::adjustForData(const Grid2d &data)
{
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].adjustForData(data);
}

/*----------------------------------------------------------------*/
void LineList::extrapolate(double seconds)
{
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].extrapolate(seconds);
}

/*----------------------------------------------------------------*/
void LineList::averageSpeeds(double speed_smoothing_len)
{
  vector<double> s, snew, len;
  vector<double>::iterator it;
  double ss;
  int i, n;
  Line *v;
    
  n = num();
  if (n <= 1)
    return;
    
  for (i=0; i<n; ++i)
  {
    v = ithLinePtr(i);
    double sp;
    if (!v->getMotionSpeed(sp))
      sp = 0.0;
    s.push_back(sp);
    len.push_back(v->length());
  }

  for (i=0; i<n; ++i)
  {
    ss = _averageSpeeds1(s, len, i, n, speed_smoothing_len);
    snew.push_back(ss);
  }
  for (i=0,it=snew.begin(); i<n; ++i,++it)
  {
    v = ithLinePtr(i);
    v->adjustVelWithHandedness(*it);
  }
}

/*----------------------------------------------------------------*/
void LineList::toGrid(Grid2d &image, double value) const
{
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].toGrid(image, value);
}

/*----------------------------------------------------------------*/
void LineList::removeSmall(bool reconnect)
{
  double small = Math::smallValue();
  removeSmall(small, reconnect);
}

/*----------------------------------------------------------------*/
void LineList::removeSmall(const double small, bool reconnect)
{
  /*
   * Get first big line as starting thing
   * Add that to the list
   */
  LineList lnew;
  int i0, i;
  Line t0;
  for (i0=-1,i=0; i<num(); ++i)
  {
    t0 = ithLine(i);
    if (t0.length() < small)
      continue;
    // no previous non-small.. append to new list.
    lnew.append(t0);
    i0 = i;
    break;
  }
  if (i0 == -1)
  {
    // no non-small lines..all are small.
    *this = lnew;
    return;
  }

  /*
   * Check each line beyond starting line
   */
  for (i=i0+1 ;i<num(); ++i)
  {
    Line t1 = ithLine(i);
    if (t1.length() < small)
      continue;
    // ith is not small
    lnew._removeSmall1(i0, i, t0, t1);
    i0 = i;
    t0 = t1;
  }

  if (reconnect)
    lnew._reconnectEndpoints();
  *this = lnew;
}

/*----------------------------------------------------------------*/
bool LineList::getMaxMaxDataValue(int &index, double &max) const
{
  double a0;
  max = 0.0;
  bool first = true;
    
  index = -1;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (_line[i].getMaxDataAtt(a0))
    {
      if (first)
      {
	first = false;
	max = a0;
	index = i;
      }
      else
      {
	if (a0 > max)
	{
	  max = a0;
	  index = i;
	}
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool LineList::maxAveAtMaxDataValue(double maxv, int &index,
				     double &ave) const
{
  double a0;
  bool first = true;
    
  ave = 0.0;
  index = -1;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (!_line[i].getMaxDataAtt(a0))
      continue;
    if (a0 != maxv)
      continue;
    if (!_line[i].getAverageDataAtt(a0))
      continue;
    if (first)
    {
      first = false;
      ave = a0;
      index = i;
    }
    else
    {
      if (a0 > ave)
      {
	ave = a0;
	index = i;
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
void LineList::order(void)
{
  int i;
  double thresh;

  if (num() <= 1)
    return;

  thresh = 0.5;
  LineList lnew;

  /*
   * Take 0th and 1th line and figure out whether
   * want to reverse 0'th or 1th lines endpoints or both
   * Do the reversing specified, add to new list.
   */
  Line t0 = ithLine(0);
  Line t1;
  if (!_reversePair(1, t0, true, thresh, lnew, t1))
  {
    clear();
    return;
  }

  /*
   * repeat this procedure for the remaining lines
   */
  for (i=2; i<num(); ++i)
  {
    /*
     * Move up one position
     */
    t0 = t1;
    if (!_reversePair(i, t0, false, thresh, lnew, t1))
    {
      LOG(ERROR) << "reversing endpoints, endpt already in place";
      clear();
      return;
    }
  }

  /*
   * Now remove zero length stuff
   */
  lnew.removeSmall(false);
  *this = lnew;
}

/*----------------------------------------------------------------*/
int LineList::minimumIndex(void) const
{
  if (num() < 0)
    return -1;
  if (num() == 1)
    return 0;
  return indexToMinimum(_isOrientedVertical());
}

/*----------------------------------------------------------------*/
int LineList::maximumIndex(void) const
{
  if (num() < 0)
    return -1;
  if (num() == 1)
    return 0;
  return indexToMaximum(_isOrientedVertical());
}

/*----------------------------------------------------------------*/
int LineList::indexToMinimum(bool is_y) const
{
  double v, v0=0.0;
  int index;
  bool first;
  
  index = -1;
  first = true;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (is_y)
      v = _line[i].minY();
    else
      v = _line[i].minX();
    if (first)
    {
      first = false;
      v0 = v;
      index = i;
    }
    else
    {
      if (v < v0)
      {
	v0 = v;
	index = i;
      }
    }
  }
  return index;
}

/*----------------------------------------------------------------*/
int LineList::indexToMaximum(bool is_y) const
{
  double v, v0=0.0;
  int index;
  bool first;
  
  index = -1;
  first = true;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (is_y)
      v = _line[i].maxY();
    else
      v = _line[i].maxX();
    if (first)
    {
      first = false;
      v0 = v;
      index = i;
    }
    else
    {
      if (v > v0)
      {
	v0 = v;
	index = i;
      }
    }
  }
  return index;
}

/*----------------------------------------------------------------*/
int LineList::indexClosestToLine(const Line &line) const
{
  int jmin = -1;
  double d, min=0.0;
    
  // find the closest one.
  for (int j=0; j<num(); ++j)
  {
    Line l = ithLine(j);
    d = line.minimumDistanceBetween(l);
    if (j == 0)
    {
      jmin = 0;
      min = d;
    }
    else
    {
      if (d < min)
      {
	jmin = j;
	min = d;
      }
    }
  }
  return jmin;
}

/*----------------------------------------------------------------*/
void LineList::removeRedundancies(void)
{
  vector<Line>::iterator it, it2;
  
  for (it=_line.begin(); it != _line.end(); ++it)
  {
    // take average over all that match in the input list
    // as regards attributes, return that list.
    AttributesEuclid a = _averageMatchAttributes(*it);
 
    int ipt = it - _line.begin();
    AttributesEuclid *ia = dynamic_cast<AttributesEuclid *>(&_line[ipt]);
    if (ia)
    {
       *ia = a;

       // remove all redundant things now.
       for (it2=it+1; it2 != _line.end();)
       {
         if (it->equalNoAttributes(*it2))
	   it2 = _line.erase(it2);
         else
	   ++it2;
       } 
    }// end if
  } // end for 
}


/*----------------------------------------------------------------*/
void LineList::setIndividualVel(double vx, double vy)
{
  MotionVector mv(vx, vy);

  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].setMotionVector(mv);
}

/*----------------------------------------------------------------*/
void LineList::setIndividualQuality(const double quality)
{
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].setQuality(quality);
}

/*----------------------------------------------------------------*/
bool LineList::averageSpeedQualityDir(bool is_fast,
				      double &speed,
				      double &quality,
				      double &dx, double &dy) const
{
  if (is_fast)
    return _averageSpeedQualityDir(speed, quality, dx, dy);
  else
    return _averageNonzeroSpeedQualityDir(speed, quality, dx, dy);
}

/*----------------------------------------------------------------*/
void LineList::nonMissingVel(void)
{
  vector<Line>::iterator i;
  for (i=_line.begin(); i!=_line.end(); )
  {
    MotionVector mv;
    if (i->getMotionVector(mv))
      ++i;
    else
      i = _line.erase(i);
  }
}

/*----------------------------------------------------------------*/
void LineList::missingVel(void)
{
  vector<Line>::iterator i;
  for (i=_line.begin(); i!=_line.end(); )
  {
    double vx, vy;
    if (i->getMotionX(vx) && i->getMotionY(vy))
      i = _line.erase(i);
    else
      ++i;
  }
}

/*----------------------------------------------------------------*/
bool LineList::meanVariance(double &vx, double &xvar, double &vy,
			     double &yvar, double &speed,
			     double &speed_var, double minq) const
{
  double num, x, vx0, vy0, q, qsum;
  num = vx = vy = speed = qsum = 0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (!_line[i].getQuality(q))
      continue;
    if (q < minq)
      continue;
    MotionVector mv;
    if (!_line[i].getMotionVector(mv))
      continue;
    vx0 = mv.getVx();
    vy0 = mv.getVy();
    vx += vx0*q;
    vy += vy0*q;
    speed += sqrt(vx0*vx0 + vy0*vy0)*q;
    qsum += q;
    ++num;
  }
  if (num <= 0)
    return false;
  if (Math::verySmall(qsum))
    return false;
  vx = vx/qsum;
  vy = vy/qsum;
  speed = speed/qsum;
  
  xvar = yvar = speed_var = 0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (!_line[i].getQuality(q))
      continue;
    if (q < minq)
      continue;
    MotionVector mv;
    if (!_line[i].getMotionVector(mv))
      continue;
    vx0 = mv.getVx();
    vy0 = mv.getVy();
    xvar += (vx0 - vx)*(vx0 - vx)*q;
    yvar += (vy0 - vy)*(vy0 - vy)*q;
    x = sqrt(vx0*vx0 + vy0*vy0);
    speed_var += (x - speed)*(x- speed)*q;
  }
  xvar = xvar/qsum;
  yvar = yvar/qsum;
  speed_var = speed_var/qsum;
  xvar = sqrt(xvar);
  yvar = sqrt(yvar);
  speed_var = sqrt(speed_var);
  return true;
}

/*----------------------------------------------------------------*/
bool LineList::qualWeightedMotion(MotionVector &mv) const
{
  double q, vx, vy;
  vx = vy = q = 0.0;
    
  for (int i=0; i<(int)_line.size(); ++i)
  {
    MotionVector mv;
    double q0;
    if (_line[i].getQuality(q0) &&
	_line[i].getMotionVector(mv))
    {
      vx += mv.getVx()*q0;
      vy += mv.getVy()*q0;
      q += q0;
    }
  }
  if (!Math::verySmall(q))
  {
    mv = MotionVector(vx/q, vy/q);
    return true;
  }
  else
  {
    mv = MotionVector(0,0);
    return false;
  }
}

/*----------------------------------------------------------------*/
bool LineList::bestLocalSpeed(const Line &o,
				double maxdist,
				double &s) const
{
  int i, n;
  double d, dist=0.0, speed;
  bool first = true;
  
  // first try best local within maxdist.
  if (_bestLocalSpeed(o, maxdist, s))
    return true;
    
  // that failed.  Instead get closest thing to o without regard
  // to distance with a vel and use that.
  if ((n=num()) <= 0)
    return false;

  speed = 0.0;
  for (i=0; i<n; ++i)
  {
    if (!_line[i].getMotionSpeed(s))
      continue;
    d = _line[i].minDistanceSquared(o);
    if (first)
    {
      dist = d;
      speed = s;
      first = false;
    }
    else
    {
      if (d < dist)
      {
	speed = s;
	dist = d;
      }
    }
  }
  if (first)
    return false;
  else
  {
    s = speed;
    return true;
  }
}

/*----------------------------------------------------------------*/
bool LineList::hasCommonLines(const LineList &m) const
{
  int i, j;
  for (i=0; i<num(); ++i)
  {
    Line li = ithLine(i);
    for (j=0; j<m.num(); ++j)
    {
      Line lj = m.ithLine(j);
      if (li.equalNoAttributes(lj))
      {
	return true;
      }
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
bool LineList::averageLocalAngleDiff(const LineList &mj,
					double &ave) const
{
  int i, j;
  double a, n;
    
  ave = n = 0.0;
  for (i=0; i<num(); ++i)
  {
    Line li = ithLine(i);
    MotionVector vi;
    if (!li.getMotionVector(vi))
      continue;
    for (j=0; j<mj.num(); ++j)
    {
      Line lj = mj.ithLine(j);
      if (!li.equalNoAttributes(lj))
	continue;
      MotionVector vj;
      if (lj.getMotionVector(vj))
      {
	a = vi.angleBetween(vj);
	ave += a*lj.length();
	n += lj.length();
      }
      break;
    }
  }
  if (n > 0.0)
  {
    ave = ave/n;
    return true;
  }
  // try to change to overall motion if possible
  MotionVector v, vj;
  if (getMotionVector(v))
  {
    if (mj.getMotionVector(vj))
    {
      ave = v.angleBetween(vj);
      return true;
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
bool
LineList::averageLocalAngleDiffIsBad(const LineList &mj, double max_diff) const
{
  int i, j;
  double a, lgood, lbad;
    
  lgood = lbad = 0.0;
  for (i=0; i<num(); ++i)
  {
    Line li = ithLine(i);
    MotionVector vi;
    if (!li.getMotionVector(vi))
      continue;
    for (j=0; j<mj.num(); ++j)
    {
      Line lj = mj.ithLine(j);
      if (!li.equalNoAttributes(lj))
	continue;
      MotionVector vj;
      if (lj.getMotionVector(vj))
      {
	a = vi.angleBetween(vj);
	if (a <= max_diff)
	  lgood += lj.length();
	else
	  lbad += lj.length();
      }
      break;
    }
  }

  if (lgood > 0.0 && lbad == 0.0)
    return false;
  else if (lgood == 0.0 && lbad > 0.0)
    return true;
  else if (lgood > 0.0 && lbad > 0.0)
    return lgood < lbad;
  else
  {
    MotionVector v, vj;
    if (getMotionVector(v) && mj.getMotionVector(vj))
    {
      a = vj.angleBetween(v);
      return a > max_diff;
    }
    else
      return false;
  }
}

/*----------------------------------------------------------------*/
bool LineList::averageLocalSpeedDiff(const LineList &mj, double &ave) const
{
  int i, j;
  double n, si, sj, s;
    
  ave = n = 0.0;
  for (i=0; i<num(); ++i)
  {
    Line li = ithLine(i);
    if (!li.getMotionSpeed(si))
      continue;
    for (j=0; j<mj.num(); ++j)
    {
      Line lj = mj.ithLine(j);
      if (!li.equalNoAttributes(lj))
	continue;
      if (lj.getMotionSpeed(sj))
      {
	s = si - sj;
	ave += s*lj.length();
	n += lj.length();
      }
      break;
    }
  }
  if (n > 0.0)
  {
    ave /= n;
    return true;
  }
  if (getMotionSpeed(si) && mj.getMotionSpeed(sj))
  {
    ave = si - sj;
    return true;
  }
  return ave;
}

/*----------------------------------------------------------------*/
void LineList::reverseHandedness(void)
{
  // reverse overall handedness. too, and individual.
  AttributesEuclid::reverseMotionHandedness();
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].reverseMotionHandedness();
}

/*----------------------------------------------------------------*/
void LineList::_init(void)
{
  _line.clear();
}

/*----------------------------------------------------------------*/
void LineList::_removeUp(double len)
{
  double total, lengthi, keep, pkeep;
  int i;

  total = 0.0;
  for (i=0; i<num(); ++i)
  {
    Line *l = ithLinePtr(i);
    lengthi = l->length();
    total += lengthi;
    keep = total - len;
    if (keep >= 0)
    {
      pkeep = keep/lengthi;
      // remove 0 through i-1'th lines, then keep pkeep near 1th
      // endpoint and all the remaining lines...start by adjusting
      // this one line, then remove the ones to be removed..
      l->bisect(1, pkeep);
      if (i > 0)
	removeElements(0, i-1);
      return;
    }
  }
  // the whole list too short..remove it all
  clear();
}

/*----------------------------------------------------------------*/
void LineList::_removeDown(double len)
{
  double total, lengthi, keep, pkeep;
  int i;

  total = 0.0;
  for (i=num()-1; i>=0; --i)
  {
    Line *l = ithLinePtr(i);
    lengthi = l->length();
    total += lengthi;
    keep = total - len;
    if (keep >= 0)
    {
      pkeep = keep/lengthi;
      l->bisect(0, pkeep);
      if (i < num()-1)
	removeElements(i+1, num()-1);
      return;
    }
  }

  // here need to remove the whole lot.
  clear();
}

/*----------------------------------------------------------------*/
vector<pair<double,double> >
LineList::_createDataweightVector(int x, int y,
				  const string &attribute_name,
				  int radius) const
{
  vector<pair<double,double> > o;
  for (int i=0; i<(int)_line.size(); ++i)
    _line[i].appendOrientationsToVector(x, y, radius*radius, attribute_name,
					   o);
  return o;
}

/*----------------------------------------------------------------*/
// take contents of l0 and connect in l1 reversed..with an expected
// common point at one end.  Return the combination list.
// case where object and l both have non-0 length.
LineList LineList::_connectOpposingNonTrivial(const LineList &l) const
{
  Line l00, l01, l10, l11;
  double x00, y00, x01, y01, x10, y10, x11, y11;

  // figure out which point is common.  It should be either the
  // 0th or last point from the two input linelists.
  l00 = ithLine(0);
  l01 = ithLine(num()-1);
  l10 = l.ithLine(0);
  l11 = l.ithLine(l.num()-1);
  l00.point(0, x00, y00);
  l01.point(1, x01, y01);
  l10.point(0, x10, y10);
  l11.point(1, x11, y11);
  if (x00 == x11 && y00 == y11)
  {
    // 0th line from this connects to last line from l...append this
    // to l.
    LineList l2(l);
    l2.appendToList(*this);
    return l2;
  }
  else if (x00 == x10 && y00 == y10)
  {
    // 0th line from this connects to 0th line from l. reverse l
    // and append this to it.
    LineList l2(l);
    l2.reverseOrder();
    l2.appendToList(*this);
    return l2;
  }
  else if (x01 == x11 && y01 == y11)
  {
    // last line from this connects to last line from l. reverse l
    // and append it to this.
    LineList l0(l);
    l0.reverseOrder();
    LineList l2(*this);
    l2.appendToList(l0);
    return l2;
  }
  else if (x01 == x10 && y01 == y10)
  {
    // last line from this connects to 0th line from l. append l to this.
    LineList l2(*this);
    l2.appendToList(l);
    return l2;
  }
  else
  {
    // error. just return the longer of the two lists.
    LOG(ERROR) << "in inputs to connect opposing lines..";
    if (num() > l.num())
      return *this;
    else
      return l;
  }
}

/*----------------------------------------------------------------*/
bool LineList::_ends(double &x0, double &y0, double &x1, double &y1) const
{
  if (num() <= 0)
  {
    return false;
  }
  Line line = ithLine(0);
  line.point(0, x0, y0);
  line = ithLine(num()-1);
  line.point(1, x1, y1);
  return true;
}


/*----------------------------------------------------------------*/
/*
 * Copy removing zero length, 1 step
 */
void LineList::_removeSmall1(int i0, int i1, const Line &t0, const Line &t1)
{
  double x0, y0, x1, y1;
  Line line2;
    
  line2 = t1;
  if (i1 > i0+1)
  {
    /*
     * A gap of 1 or more small lines..Get 1th endpoint of
     * last line added as 0th, and 1th endpt of current line 
     * as 1th, build copy of viline with this new line.
     */
    t0.point(1, x0, y0);
    t1.point(1, x1, y1);
	
    // change line2 endpoints
    line2.adjustEndpoints(x0, y0, x1, y1);
  }

  /*
   * Add that in 
   */
  append(line2);
}

/*----------------------------------------------------------------*/
void LineList::_reconnectEndpoints(void)
{
  int i;
  Line *l0, *l1;
  double x00, x01, y00, y01, x10, x11, y10, y11, newx, newy;
    
  if (num() < 2)
    return;
    
  /*
   * Go through and change endpoints to reconnect
   */
  for (i=0; i < num()-1; ++i)
  {
    l0 = ithLinePtr(i);
    l1 = ithLinePtr(i+1);
    l0->point(0, x00, y00);
    l0->point(1, x01, y01);
    l1->point(0, x10, y10);
    l1->point(1, x11, y11);
    newx = (x01 + x10)/2.0;
    newy = (y01 + y10)/2.0;
    // change endpoints
    l0->adjustEndpoints(x00, y00, newx, newy);
    l1->adjustEndpoints(newx, newy, x11, y11);
  }        
}



/*----------------------------------------------------------------*/
/*
 * Return the higher index line that may or may not be reversed
 */
bool
LineList::_reversePair(int high_index,
			Line &l0, /* lower index line copy , passed in*/
			bool use_zero,
			double thresh,
			LineList &lnew,
			Line &l1) const
{
  bool reverse0, reverse1;
    
  /*
   * get high index line copy.
   */
  l1 = ithLine(high_index);

  /*
   * Check for reversal.
   */
  l0.checkForEndpointReverse(l1, thresh, reverse0, reverse1);
  if (reverse0)
  {
    if (use_zero)
      l0.reverse();
    else
    {
      /*
       * Want to reverse 0th line, but its already oriented properly
       * relative to the previous line
       */
      LOG(ERROR) << "reversing endpoints, endpt already in place";
      return false;
    }
  }
  if (reverse1)
    l1.reverse();

  if (use_zero)
  {
    /*
     * Add in 0th to new list
     */
    lnew.append(l0);
  }

    
  /*
   * Add in 1th to new list
   */
  lnew.append(l1);
  return true;
}

/*----------------------------------------------------------------*/
bool LineList::_isOrientedVertical(void) const
{
  double a;
  if (averageOrientation(a))
  {
    return fabs(a) > 45.0;
  }
  else
  {
    return false;
  }
}


/*----------------------------------------------------------------*/
// take average over all __elements that match o in the input list
// as regards attributes, return that list.
// modified to take maximum time by dave albo.
AttributesEuclid LineList::_averageMatchAttributes(const Line &o) const
{
  vector<AttributesEuclid> v;

  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (_line[i].equalNoAttributes(o))
      v.push_back(_line[i]);
  }
  return AttributesEuclid(v);
}

/*----------------------------------------------------------------*/
bool LineList::_averageSpeedQualityDir(double &speed,
				       double &quality,
				       double &dx, double &dy) const
{
  double n, q, vx, vy, s, ssum, q0, vx0, vy0, qret, nq;

  dx = dy = quality = speed = 0.0;
  n = q = vx = vy = ssum = qret = nq = 0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (!_line[i].getQuality(q0))
      continue;
    qret += q0;
    nq += 1.0;
    if (!_line[i].getMotionX(vx0))
      continue;
    if (!_line[i].getMotionY(vy0))
      continue;
    s = sqrt(vx0*vx0 + vy0*vy0);
    n++;
    q += q0;
    if (!Math::verySmall(s))
    {
      vx += vx0*q0/s;
      vy += vy0*q0/s;
      ssum += s*q0;
    }
  }
  if (nq == 0.0)
    return 0;
  quality = qret/nq;
  if (n == 0.0 || Math::verySmall(q))
  {
    // can't set the speed.
    dx = dy = speed = 0.0;
    return false;
  }

  if (Math::verySmall(q))
    return false;
  dx = vx/q;
  dy = vy/q;
  speed = ssum/q;
  return true;
}

/*----------------------------------------------------------------*/
// get the average non-missing non-zero speed and direction,
// and the average non-missing quality
// return 1 if computations were successful.
bool LineList::_averageNonzeroSpeedQualityDir(double &speed,
					      double &quality,
					      double &dx,
					      double &dy) const
{
  double n, q, vx, vy, s, ssum, q0, vx0, vy0, qret, nq;

  dx = dy = quality = speed = 0.0;
  n = q = vx = vy = ssum = qret = nq = 0.0;
  for (int i=0; i<(int)_line.size(); ++i)
  {
    if (!_line[i].getQuality(q0))
      continue;
    qret += q0;
    nq += 1.0;
    if (!_line[i].getMotionX(vx0))
      continue;
    if (!_line[i].getMotionY(vy0))
      continue;
    s = sqrt(vx0*vx0 + vy0*vy0);
    if (!Math::verySmall(s))
    {
      n++;
      q += q0;
      vx += vx0*q0/s;
      vy += vy0*q0/s;
      ssum += s*q0;
    }
  }
  if (nq == 0.0)
    return false;
  quality = qret/nq;
  if (n == 0.0)
  {
    dx = dy = speed = 0.0;
    return true;
  }

  if (Math::verySmall(q))
    return false;
  dx = vx/q;
  dy = vy/q;
  speed = ssum/q;
  return true;
}


/*----------------------------------------------------------------*/
// return speed based on average of elements close to o in list.
bool LineList::_bestLocalSpeed(const Line &o, double maxdist,
			       double &ret) const
{
  int i, n;
  double d, speed, ns, s, m;
    
  if ((n=num()) <= 0)
    return false;

  ns = speed = 0.0;
  m = maxdist*maxdist;
  for (i=0; i<n; ++i)
  {
    d = _line[i].minDistanceSquared(o);
    if (d > m)
      continue;
    if (!_line[i].getMotionSpeed(s))
      continue;
    speed += s;
    ns++;
  }
  if (ns == 0.0)
    return false;
  else
  {
    ret = speed/ns;
    return true;
  }
}

