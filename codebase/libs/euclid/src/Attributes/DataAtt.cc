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
#include <euclid/DataAtt.hh>
#include <euclid/Grid2d.hh>
#include <euclid/PointList.hh>

using std::string;


/*----------------------------------------------------------------*/
DataAtt::DataAtt()
{ 
  _min =  _max =  _ave =  _median = 254.0;
  _npt = 10000.0;
}

/*----------------------------------------------------------------*/
DataAtt::DataAtt(double mind, double maxd, double med, double aved,
		 double nptd)
{
  _min = mind;
  _max = maxd;
  _ave = aved;
  _median = med;
  _npt = nptd;
}

/*----------------------------------------------------------------*/
DataAtt::DataAtt(const PointList &l, const Grid2d &data)
{
  _fillFromList(l, data);
}

/*----------------------------------------------------------------*/
DataAtt::~DataAtt()
{
}

/*----------------------------------------------------------------*/
bool DataAtt::operator==(const DataAtt &v) const
{
  return (_min == v._min &&
	  _max == v._max &&
	  _ave == v._ave &&
	  _median == v._median &&
	  _npt == v._npt);
}

/*----------------------------------------------------------------*/
void DataAtt::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void DataAtt::print(FILE *fp) const
{
  fprintf(fp, "%s", sprint().c_str());
}

/*----------------------------------------------------------------*/
string DataAtt::sprint(void) const
{
  char buf[100];
  sprintf(buf, "d:(min:%.2f max:%.2f ave:%.2f npt:%.2f)",
	  _min, _max, _ave, _npt);
  string ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
bool DataAtt::hasData(double min_ave, double min_max) const
{
  return (_ave > min_ave && _max > min_max);
}

/*----------------------------------------------------------------*/
// note that this takes missing values and averages them in as 0
void DataAtt::_fillFromList(const PointList &l, const Grid2d &data)
{
  int i;
  double sum;
    
  /*
   * Don't compute median for now, but do
   * compute everything else.
   */
  _min = _max = _median = _ave = _npt = 0.0;
  sum = 0.0;

  for (i=0; i<l.size(); ++i)
  {
    int x = (int)l.ithX(i);
    int y = (int)l.ithY(i);
    double v;
    if (!data.getValue(x, y, v))
      v = 0.0;
    if (i == 0)
      _min = _max = v;
    else
    {
      if (v > _max)
	_max = v;
      if (v < _min)
	_min = v;
    }
    sum += v;
    ++_npt;
  }

  if (_npt > 0)
    _ave = sum/_npt;
  return;
}

