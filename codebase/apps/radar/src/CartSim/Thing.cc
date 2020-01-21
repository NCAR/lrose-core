// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file Thing.cc
 */

#include "Thing.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
static void _sSetEndpoints(const int n, const Params::Xy_t *v,
			   vector<Xyz> &pts)
{
  for (int i=0; i<n; ++i)
  {
    Xyz pt(v[i].x, v[i].y, 0);
    pt.scale(KM_TO_METERS);
    pts.push_back(pt);
  }
}

//----------------------------------------------------------------
static void _sSetFuzzy(const double xscale, const int n,
		       const Params::Xy_t *v,  FuzzyF &f)
{
  vector<pair<double, double> > p;
  for (int i=0; i<n; ++i)
  {
    p.push_back(pair<double,double>(v[i].x*xscale, v[i].y));
  }
  f = FuzzyF(p);
}

//----------------------------------------------------------------
Thing::Thing(const Params &P, int minutes2intensity_index,
	     int minutes2size_index, double startMin, double lifetimeMin) :
  Lifetime(startMin, lifetimeMin), _intensity(0)
{
  if (minutes2intensity_index >= 0)
  {
    _setFuzzyMinutesToInterest(P, minutes2intensity_index, _seconds2intensity);
  }
  else
  {
    // assume its a fuzzy function 1 for all time
    vector<pair<double,double> > xy;
    xy.push_back(pair<double,double>(0, 1));
    xy.push_back(pair<double,double>(10000, 1));
    _seconds2intensity = FuzzyF(xy);
  }

  if (minutes2size_index >= 0)
  {
    _setFuzzyMinutesToInterest(P, minutes2size_index, _seconds2size);
  }
  else
  {
    // assume its a fuzzy function 1 for all time
    vector<pair<double,double> > xy;
    xy.push_back(pair<double,double>(0, 1));
    xy.push_back(pair<double,double>(10000, 1));
    _seconds2size = FuzzyF(xy);
  }
}

//----------------------------------------------------------------
Thing::~Thing()
{
}

//----------------------------------------------------------------
void Thing::addToData(const int seconds, const Xyz &loc, Data &data)
{
  if (!_setIntensity(seconds))
  {
    return;
  }
  _elapsedSeconds = Lifetime::elapsedSeconds(seconds);
  _addToData(loc, data);
}

//----------------------------------------------------------------
bool Thing::endpoints(const Params &P, int index, std::vector<Xyz> &loc)
{
  switch (index)
  {
  case 0:
    _sSetEndpoints(P.endpoint0_n, P._endpoint0, loc);
    break;
  case 1:
    _sSetEndpoints(P.endpoint1_n, P._endpoint1, loc);
    break;
  case 2:
    _sSetEndpoints(P.endpoint2_n, P._endpoint2, loc);
    break;
  case 3:
    _sSetEndpoints(P.endpoint3_n, P._endpoint3, loc);
    break;
  case 4:
    _sSetEndpoints(P.endpoint4_n, P._endpoint4, loc);
    break;
  case 5:
    _sSetEndpoints(P.endpoint5_n, P._endpoint5, loc);
    break;
  case 6:
    _sSetEndpoints(P.endpoint6_n, P._endpoint6, loc);
    break;
  case 7:
    _sSetEndpoints(P.endpoint7_n, P._endpoint7, loc);
    break;
  case 8:
    _sSetEndpoints(P.endpoint8_n, P._endpoint8, loc);
    break;
  case 9:
    _sSetEndpoints(P.endpoint9_n, P._endpoint9, loc);
    break;
  case 10:
    _sSetEndpoints(P.endpoint10_n, P._endpoint10, loc);
    break;
  case 11:
    _sSetEndpoints(P.endpoint11_n, P._endpoint11, loc);
    break;
  default:
    printf("Endpoint index %d out of range 0 to 11\n", index);
    return false;
  }
  return true;
}

//----------------------------------------------------------------
bool Thing::_setIntensity(const int seconds)
{
  if (Lifetime::outOfRange(seconds))
  {
    _intensity = 0;
  }
  else
  {
    _intensity = _seconds2intensity.apply(Lifetime::elapsedSeconds(seconds));
  }

  return _intensity > 0;
}

//----------------------------------------------------------------
void Thing::_setFuzzyMinutesToInterest(const Params &P, const int i,  FuzzyF &f)
{
  switch (i)
  {
  case 0:
    _sSetFuzzy(60.0, P.fuzzy0_n, P._fuzzy0, f);
    break;
  case 1:
    _sSetFuzzy(60.0, P.fuzzy1_n, P._fuzzy1, f);
    break;
  case 2:
    _sSetFuzzy(60.0, P.fuzzy2_n, P._fuzzy1, f);
    break;
  case 3:
    _sSetFuzzy(60.0, P.fuzzy3_n, P._fuzzy3, f);
    break;
  case 4:
    _sSetFuzzy(60.0, P.fuzzy4_n, P._fuzzy4, f);
    break;
  case 5:
    _sSetFuzzy(60.0, P.fuzzy5_n, P._fuzzy5, f);
    break;
  case 6:
    _sSetFuzzy(60.0, P.fuzzy6_n, P._fuzzy6, f);
    break;
  case 7:
    _sSetFuzzy(60.0, P.fuzzy7_n, P._fuzzy7, f);
    break;
  case 8:
    _sSetFuzzy(60.0, P.fuzzy8_n, P._fuzzy8, f);
    break;
  case 9:
    _sSetFuzzy(60.0, P.fuzzy9_n, P._fuzzy9, f);
    break;
  case 10:
    _sSetFuzzy(60.0, P.fuzzy10_n, P._fuzzy10, f);
    break;
  case 11:
    _sSetFuzzy(60.0, P.fuzzy11_n, P._fuzzy11, f);
    break;
  default:
    printf("Fuzzy index %d out of range 0 to 11\n", i);
    exit(1);
  }
}

//----------------------------------------------------------------
void Thing::_setFuzzyKmToInterest(const Params &P, const int i,  FuzzyF &f)
{
  switch (i)
  {
  case 0:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy0_n, P._fuzzy0, f);
    break;
  case 1:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy1_n, P._fuzzy1, f);
    break;
  case 2:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy2_n, P._fuzzy1, f);
    break;
  case 3:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy3_n, P._fuzzy3, f);
    break;
  case 4:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy4_n, P._fuzzy4, f);
    break;
  case 5:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy5_n, P._fuzzy5, f);
    break;
  case 6:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy6_n, P._fuzzy6, f);
    break;
  case 7:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy7_n, P._fuzzy7, f);
    break;
  case 8:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy8_n, P._fuzzy8, f);
    break;
  case 9:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy9_n, P._fuzzy9, f);
    break;
  case 10:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy10_n, P._fuzzy10, f);
    break;
  case 11:
    _sSetFuzzy(KM_TO_METERS, P.fuzzy11_n, P._fuzzy11, f);
    break;
  default:
    printf("Fuzzy index %d out of range 0 to 11\n", i);
    exit(1);
  }
}

//----------------------------------------------------------------
void Thing::_setFuzzy(const Params &P, const int i,  FuzzyF &f)
{
  switch (i)
  {
  case 0:
    _sSetFuzzy(1.0, P.fuzzy0_n, P._fuzzy0, f);
    break;
  case 1:
    _sSetFuzzy(1.0, P.fuzzy1_n, P._fuzzy1, f);
    break;
  case 2:
    _sSetFuzzy(1.0, P.fuzzy2_n, P._fuzzy1, f);
    break;
  case 3:
    _sSetFuzzy(1.0, P.fuzzy3_n, P._fuzzy3, f);
    break;
  case 4:
    _sSetFuzzy(1.0, P.fuzzy4_n, P._fuzzy4, f);
    break;
  case 5:
    _sSetFuzzy(1.0, P.fuzzy5_n, P._fuzzy5, f);
    break;
  case 6:
    _sSetFuzzy(1.0, P.fuzzy6_n, P._fuzzy6, f);
    break;
  case 7:
    _sSetFuzzy(1.0, P.fuzzy7_n, P._fuzzy7, f);
    break;
  case 8:
    _sSetFuzzy(1.0, P.fuzzy8_n, P._fuzzy8, f);
    break;
  case 9:
    _sSetFuzzy(1.0, P.fuzzy9_n, P._fuzzy9, f);
    break;
  case 10:
    _sSetFuzzy(1.0, P.fuzzy10_n, P._fuzzy10, f);
    break;
  case 11:
    _sSetFuzzy(1.0, P.fuzzy11_n, P._fuzzy11, f);
    break;
  default:
    printf("Fuzzy index %d out of range 0 to 11\n", i);
    exit(1);
  }
}

//----------------------------------------------------------------
void Thing::_setFuzzy(int n, const Params::Xy_t *v, FuzzyF &f)
{
  _sSetFuzzy(1.0, n, v, f);
}
