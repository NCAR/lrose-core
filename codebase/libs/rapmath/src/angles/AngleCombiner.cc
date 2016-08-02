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
#include <toolsa/copyright.h>
#include <rapmath/AngleCombiner.hh>
#include <rapmath/OrderedList.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
using std::vector;

#define BAD_ANGLE -9999.99

/*----------------------------------------------------------------*/
AngleCombiner::AngleCombiner(int n, bool is_motion_angles)
{
  int i;
  AngleConfWeight_t a;
  
  _is_set = false;
  _n = 0;
  _max_n = n;
  _angle = _conf = BAD_ANGLE;
  _is_360 = is_motion_angles;
  for (i=0; i<_max_n; ++i)
  {
    a.angle = 0.0;
    a.conf = 0.0;
    a.weight = 1.0;
    _acw.push_back(a);
  }
}

/*----------------------------------------------------------------*/
AngleCombiner::AngleCombiner(const vector<double> &w, bool is_motion_angles)
{
  int i;
  AngleConfWeight_t a;
  
  _max_n = (int)(w.size());
  _is_set = false;
  _n = 0;
  _angle = _conf = BAD_ANGLE;
  _is_360 = is_motion_angles;
  for (i=0; i<_max_n; ++i)
  {
    a.angle = 0.0;
    a.conf = 0.0;
    a.weight = *(w.begin() + i);
    _acw.push_back(a);
  }
}

/*----------------------------------------------------------------*/
AngleCombiner::~AngleCombiner()
{
}

/*----------------------------------------------------------------*/
void AngleCombiner::clearValues(void)
{
  _n = 0;
  _is_set = false;
}

/*----------------------------------------------------------------*/
void AngleCombiner::setGood(int i, double angle, double conf)
{
  AngleConfWeight_t *a;

  if (i == 0)
  {
    // assume starting over now..
    _n = 0;
    _is_set = false;
  }

  if (i >= _max_n)
  {
    LOG(ERROR) << "too many inputs to AngleCombiner " << i << " of " << _max_n;
    return;
  }
  if (i != _n)
  {
    LOG(ERROR) << "fuzzyorients not set in order:  i=" << i << " last=" << _n;
    return;
  }

  a = &(*(_acw.begin() + i));
  a->angle = angle;
  a->conf = conf;
  ++_n;
  _is_set = false;
}

/*----------------------------------------------------------------*/
void AngleCombiner::setBad(int i)
{
  AngleConfWeight_t *a;

  if (i == 0)
  {
    // assume starting over now..
    _n = 0;
    _is_set = false;
  }

  if (i >= _max_n)
  {
    LOG(ERROR) << "too many inputs to AngleCombiner " << i << " of " << _max_n;
    return;
  }
  if (i != _n)
  {
    LOG(ERROR) << "fuzzyorients not set in order:  i=" << i << " last=" << _n;
    return;
  }

  a = &(*(_acw.begin() + i));
  a->angle = BAD_ANGLE;
  a->conf = 0.0;
  ++_n;
  _is_set = false;
}

/*----------------------------------------------------------------*/
bool AngleCombiner::getCombineAngleConf(double &v, double &c)
{
  if (_n == 0)
    return false;
  if (_n == 1)
  {
    v = _acw.begin()->angle;
    c = _acw.begin()->conf;
    return true;
  }
  if (!_is_set)
  {
    _setValues();
  }
  v = _angle;
  c = _conf;
  return v != BAD_ANGLE;
}

/*----------------------------------------------------------------*/
bool AngleCombiner::getCombineAngle(double &v)
{
  if (_n == 0)
    return false;
  if (_n == 1)
  {
    v = _acw.begin()->angle;
    return true;
  }
  if (!_is_set)
  {
    _setValues();
  }
  v = _angle;
  return v != BAD_ANGLE;
}

/*----------------------------------------------------------------*/
void AngleCombiner::_setValues(void)
{
  _angle = _conf = BAD_ANGLE;
  if (_is_360)
  {
    _setMotionValues();
  }
  else
  {
    _setOrientationValues();
  }
  _is_set = true;
}

/*----------------------------------------------------------------*/
void AngleCombiner::_setOrientationValues(void)
{
  double a0, a1;
    
  // get range of azimuths, try to put within 90 degrees of each other..
  if (!_orientationRange(a0, a1))
    return;

  if (a1 - a0 > 90.0)
    // large and small values, really!
    _largeSmallCombine();
  else
    // here range of angles < 90.0..can process by simple fuzzy logic.
    _simpleCombine();
}

/*----------------------------------------------------------------*/
void AngleCombiner::_setMotionValues(void)
{
  double a0, a1;
    
  // get range of azimuths, try to put within 90 degrees of each other..
  if (!_motionRange(a0, a1))
    return;

  if (a1 - a0 > 180.0)
    // large and small values, really!
    _largeSmallCombine();
  else
    // here range of angles < 180.0..can process by simple fuzzy logic.
    _simpleCombine();
}

/*----------------------------------------------------------------*/
bool AngleCombiner::_orientationRange(double &a0, double &a1)
{
  double min;
    
  if (!_actualRange(a0, a1))
    return false;

  if (a1 - a0 <= 90.0)
    // ok as is.
    return true;

  // try adding 180 to all angles less than 90 below max
  min = a1 - 90.0;
  _adjustedRange(min, 180.0, a0, a1);
  if (a1 - a0 > 90.0)
    // the angles really were in a range > 90
    return true;

  // adding 180 did help. go ahead and add 180 to all less than the min
  _doAdjust(min, 180.0);
  return true;
}

/*----------------------------------------------------------------*/
bool AngleCombiner::_motionRange(double &a0, double &a1)
{
  double min;
    
  if (!_actualRange(a0, a1))
    return false;

  if (a1 - a0 <= 180.0)
    // ok as is.
    return true;

  // try adding 360 to all angles less than 180 below max
  min = a1 - 180.0;
  _adjustedRange(min, 360.0, a0, a1);
  if (a1 - a0 > 180.0)
    // the angles really were in a range > 180
    return true;

  // adding 360 did help. go ahead and add 360 to all below the min.
  _doAdjust(min, 360.0);
  return true;
}

/*----------------------------------------------------------------*/
bool AngleCombiner::_actualRange(double &a0, double &a1) const
{
  bool first;
  int i;
  vector<AngleConfWeight_t>::const_iterator it;
    
  for (first=true,i=0,it=_acw.begin(); i<_n; ++i,++it)
  {
    if (it->conf == 0.0)
      continue;
    if (first)
    {
      a0 = a1 = it->angle;
      first = false;
    }
    else
    {
      if (it->angle < a0)
	a0 = it->angle;
      if (it->angle > a1)
	a1 = it->angle;
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
void AngleCombiner::_adjustedRange(double min_good, double add,
				   double &a0, double &a1) const
{
  bool first;
  int i;
  double a;
  vector<AngleConfWeight_t>::const_iterator it;
    
  // add add to all angles less than min_good and compute
  // min/max from that.
  for (first=true,i=0,it=_acw.begin(); i<_n; ++i,++it)
  {
    if (it->conf == 0.0)
      continue;
    a = it->angle;
    if (a < min_good)
      a += add;
    if (first)
    {
      a0 = a1 = a;
      first = false;
    }
    else
    {
      if (a < a0)
	a0 = a;
      if (a > a1)
	a1 = a;
    }
  }
}

/*----------------------------------------------------------------*/
void AngleCombiner::_doAdjust(double min, double add)
{
  int i;
  vector<AngleConfWeight_t>::iterator it;

  for (i=0,it=_acw.begin(); i<_n; ++i,++it)
  {
    if (it->conf == 0.0)
      continue;
    if (it->angle < min)
      it->angle += add;
  }
}

/*----------------------------------------------------------------*/
void AngleCombiner::_simpleCombine(void)
{
  double sum, csum, cdenom, penalty;
  vector<AngleConfWeight_t>::iterator it;
  int i;
    
  // just take fuzzy average as weighted angle value.
  sum = csum = cdenom = 0.0;
  penalty = 0.0;
  for (i=0,it=_acw.begin(); i<_n; ++i,++it)
  {
    if (it->conf == 0.0)
    {
      penalty += 0.1;
      continue;
    }
    sum += it->angle*it->conf*it->weight;
    csum += it->conf*it->weight;
    cdenom += it->weight;
  }
  if (cdenom == 0.0 || csum == 0.0)
    return;

  _angle = sum/csum;
  _conf = csum/cdenom;

  // reduce by penalty.
  _conf *= (1.0-penalty);
  if (_conf < 0.0)
    _conf = 0.0;

  if (_is_360)
  {
    // want to put result into [-180,180]
    while (_angle > 180)
      _angle -= 360.0;
    while (_angle < -180)
      _angle += 360.0;
  }
  else
  {
    // here want to take the angles > 180 and rescale back into 0 to 180
    while (_angle >= 180.0)    
      _angle -= 180.0;
  }
}

/*----------------------------------------------------------------*/
void AngleCombiner::_largeSmallCombine(void)
{
  OrderedList o;
  int i, remove, p;
  bool good, error;
  vector<AngleConfWeight_t>::iterator it;
    
  // order w*conf.
  for (i=0,it=_acw.begin(); i<_n; ++i,++it)
    o.addToListUnordered(it->conf*it->weight);
  o.order();

  // remove from bottom till no more large_small situation
  good = false;
  for (remove=0; remove<_n-1; ++remove)
  {
    // set remove'th confidence to 0.
    p = o.ithPerm(remove);
    if (p == -1)
    {
      LOG(ERROR) << "in large_small combine";
      break;
    }
    (_acw.begin() + p)->conf = 0.0;
    if (_checkRange(error))
    {
      good = !error;
      break;
    }
    if (error)
    {
      good = false;
      break;
    }
  }

  if (!good)
    return;

  _simpleCombine();
}

/*----------------------------------------------------------------*/
bool AngleCombiner::_checkRange(bool &error)
{
  bool stat;
  double a0, a1;

  if (_is_360)
  {
    stat = _motionRange(a0, a1);
  }
  else
  {
    stat = _orientationRange(a0, a1);
  }

  if (!stat)
  {
    LOG(ERROR) << "in large_small combine";
    error = true;
    return true;
  }
  error = false;
  if (_is_360)
  {
    return (a1 - a0 <= 180.0);
  }
  else
  {
    return (a1 - a0 <= 90.0);
  }
}

/*----------------------------------------------------------------*/
bool AngleCombiner::isBadAngle(const double angle)
{
  return angle == BAD_ANGLE;
}
