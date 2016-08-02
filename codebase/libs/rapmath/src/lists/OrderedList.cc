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

#include <cstdio>
#include <rapmath/OrderedList.hh>
#include <rapmath/Math.hh>

using std::vector;
using std::pair;

/*----------------------------------------------------------------*/
// default constructor..empty list
OrderedList::OrderedList()
{
  _n = 0;
  _is_ordered = false;
  _is_ordered_weights = false;
}

/*----------------------------------------------------------------*/
// destructor
OrderedList::~OrderedList()
{
  clear();
}

/*----------------------------------------------------------------*/
void OrderedList::print(void) const
{
  vector<pair<double,int> >::const_iterator i;
  
  printf("List:");
  for (i=_val.begin(); i!= _val.end(); ++i)
    printf(" %.3lf", i->first);
  printf("Weights:");
  for (i=_w.begin(); i!= _w.end(); ++i)
    printf(" %.3lf", i->first);
  printf("\n");
}
  
/*----------------------------------------------------------------*/
void OrderedList::clear(void)
{
  _n = 0;
  _val.clear();
  _w.clear();
  _is_ordered = false;
  _is_ordered_weights = false;
}

/*----------------------------------------------------------------*/
void OrderedList::addToListUnordered(double v)
{
  pair<double,int> d(v, -1);

  _val.push_back(d);
  d.first = 1.0;
  _w.push_back(d);
  ++_n;
  _is_ordered = false;
  _is_ordered_weights = false;
}

/*----------------------------------------------------------------*/
void OrderedList::addToListUnordered(const std::pair<double,double> &dw)
{
  pair<double,int> d(dw.first, -1);
  _val.push_back(d);
  d.first = dw.second;
  _w.push_back(d);
  ++_n;
  _is_ordered = false;
  _is_ordered_weights = false;
}

/*----------------------------------------------------------------*/
void OrderedList::order(void)
{
  // create interface to usort method
  Math::sort(_val);
  _is_ordered = true;
}

/*----------------------------------------------------------------*/
void OrderedList::orderWeights(void)
{
  Math::sort(_w);
  _is_ordered_weights = true;
}

/*----------------------------------------------------------------*/
double OrderedList::percentile(double p)
{
  int pcnt;
  
  if (_n <= 0)
    return 0.0;
  if (_n == 1)
    return _ithd(0);
  if (!_is_ordered)
  {
    order();
  }

  pcnt = (int)(p*(double)_n);
  if (pcnt < 0)
    pcnt = 0;
  if (pcnt >= _n)
    pcnt = _n-1;
  return (_ithd(_ithi(pcnt)));
}

/*----------------------------------------------------------------*/
double OrderedList::weightPercentile(double p)
{
  int pcnt;
  
  if (_n <= 0)
    return 0.0;
  if (_n == 1)
    return _ithd(0);
  if (!_is_ordered_weights)
  {
    orderWeights();
  }

  pcnt = (int)(p*(double)_n);
  if (pcnt < 0)
    pcnt = 0;
  if (pcnt >= _n)
    pcnt = _n-1;
  return (_ithd(_ithwi(pcnt)));
}

/*----------------------------------------------------------------*/
double OrderedList::weightConstrainedAverage(double p0, double p1)
{
  int pcnt0, pcnt1, i;
  double sum, num;

  if (_n <= 0)
    return 0.0;
  if (_n == 1)
    return _ithd(0);
  if (!_is_ordered_weights)
    orderWeights();

  pcnt0 = (int)(p0*(double)_n);
  if (pcnt0 < 0)
    pcnt0 = 0;
  if (pcnt0 >= _n)
    pcnt0 = (int)(_n-1);
  pcnt1 = (int)(p1*(double)_n);
  if (pcnt1 < 0)
    pcnt1 = 0;
  if (pcnt1 >= _n)
    pcnt1 = _n-1;

  num = sum = 0.0;
  for (i=pcnt0; i<=pcnt1; ++i)
  {
    num += 1.0;
    sum += _ithd(_ithwi(i));
  }
  if (num > 0.0)
    return sum/num;
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
// return ith ordered index (from bottom up)
int OrderedList::ithPerm(int i)
{
  if (i < 0 || i >= _n) 
    return -1;
  if (_n <= 0)
    return -1;
  if (_n == 1)
    return 0;
  if (!_is_ordered)
    order();
  return (_val.begin() + i)->second;
}

