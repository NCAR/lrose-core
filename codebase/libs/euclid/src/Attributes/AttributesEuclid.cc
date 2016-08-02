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
#include <euclid/AttributesEuclid.hh>
#include <euclid/MotionVector.hh>
#include <euclid/DataAtt.hh>
#include <cmath>
using std::vector;
using std::string;

/*----------------------------------------------------------------*/
AttributesEuclid::AttributesEuclid(void) : Attributes()
{
}

/*----------------------------------------------------------------*/
AttributesEuclid::~AttributesEuclid()
{
}

/*----------------------------------------------------------------*/
AttributesEuclid::AttributesEuclid(const vector<AttributesEuclid> &v) :
  Attributes()
{

  vector<Attributes> a;
  for (size_t i=0; i<v.size(); ++i)
  {
    a.push_back(v[i]);
  }

  vector<string> useMax;
  useMax.push_back("ID");
  useMax.push_back("TIME");
  useMax.push_back("Time");
  useMax.push_back("time"); 
  useMax.push_back("Expire_time"); 
  useMax.push_back("EXPIRE_TIME"); 
  useMax.push_back("expire_time"); 
  useMax.push_back("Extrap_seconds");
  useMax.push_back("EXTRAP_SECONDS");

  Attributes::operator=(Attributes(a, useMax));
}  

/*----------------------------------------------------------------*/
bool AttributesEuclid::getMotionVector(MotionVector &v) const
{
  double vx, vy;
  if (getDouble("Motion_x", vx) && getDouble("Motion_y", vy))
  {
    v = MotionVector(vx, vy);
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::averageMotionVector(const AttributesEuclid &a1, 
					   MotionVector &v) const
{
  if (getMotionVector(v))
  {
    MotionVector v2;
    if (a1.getMotionVector(v2))
    {
      v.average(v2);
    }
  }
  else
  {
    if (!a1.getMotionVector(v))
    {
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::getMotionSpeed(double &s) const
{
  double vx, vy;
  if (getDouble("Motion_x", vx) && getDouble("Motion_y", vy))
  {
    s = sqrt(vx*vx + vy*vy);
    return true;
  }
  else
  {
    return false;
  }
}  

/*----------------------------------------------------------------*/
bool AttributesEuclid::getMotionX(double &vx) const
{
  return getDouble("Motion_x", vx);
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::getMotionY(double &vy) const
{
  return getDouble("Motion_y", vy);
}

/*----------------------------------------------------------------*/
void AttributesEuclid::reverseMotionHandedness(void)
{
  MotionVector mv;

  if (getMotionVector(mv))
  {
    mv.reverseHandedness();
    setMotionVector(mv);
  }
}

/*----------------------------------------------------------------*/
void AttributesEuclid::setMotionVector(const MotionVector &v)
{
  double vx = v.getVx(); 
  double vy = v.getVy();
  addDouble("Motion_x", vx);
  addDouble("Motion_y", vy);
}

/*----------------------------------------------------------------*/
void AttributesEuclid::removeMotionVector(void)
{
  removeDouble("Motion_x");
  removeDouble("Motion_y");
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::getQuality(double &q) const
{
  return getDouble("Quality", q);
}
  
/*----------------------------------------------------------------*/
void AttributesEuclid::setQuality(double q)
{
  addDouble("Quality", q);
}

/*----------------------------------------------------------------*/
void AttributesEuclid::removeQuality(void)
{
  removeDouble("Quality");
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::averageQuality(const AttributesEuclid &a1,
				      double &v) const
{
  if (getQuality(v))
  {
    double v2;
    if (a1.getQuality(v2))
    {
      v = (v + v2)/2.0;
    }
  }
  else
  {
    if (!a1.getQuality(v))
    {
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::getTime(time_t &t) const
{
  double q;
  if (getDouble("Time", q))
  {
    t = static_cast<time_t>(q);
    return true;
  }
  else
  {
    return false;
  }
}
  
/*----------------------------------------------------------------*/
void AttributesEuclid::setTime(const time_t &t)
{
  double q = static_cast<double>(t);
  addDouble("Time", q);
}

/*----------------------------------------------------------------*/
void AttributesEuclid::removeTime(void)
{
  removeDouble("Time");
}


/*----------------------------------------------------------------*/
bool AttributesEuclid::getDataAtt(DataAtt &v) const
{
  double min, max, ave, median, npt;

  bool stat = true;
  if (!getDouble("DataAtt_min", min))
  {
    stat = false;
  }
  if (!getDouble("DataAtt_max", max))
  {
    stat = false;
  }
  if (!getDouble("DataAtt_ave", ave))
  {
    stat = false;
  }
  if (!getDouble("DataAtt_median", median))
  {
    stat = false;
  }
  if (!getDouble("DataAtt_npt", npt))
  {
    stat = false;
  }
  if (stat)
  {
    v = DataAtt(min, max, ave, median, npt);
  }
  return stat;
}

/*----------------------------------------------------------------*/
void AttributesEuclid::setDataAtt(const DataAtt &v)
{
  addDouble("DataAtt_min", v.getMin());
  addDouble("DataAtt_max", v.getMax());
  addDouble("DataAtt_ave", v.getAve());
  addDouble("DataAtt_median", v.getMedian());
  addDouble("DataAtt_npt", v.getNpt());
  addDouble("DataAtt_min", v.getMin());
}

/*----------------------------------------------------------------*/
void AttributesEuclid::removeDataAtt(void)
{
  removeDouble("DataAtt_min");
  removeDouble("DataAtt_max");
  removeDouble("DataAtt_ave");
  removeDouble("DataAtt_median");
  removeDouble("DataAtt_npt");
  removeDouble("DataAtt_min");
}

/*----------------------------------------------------------------*/
bool AttributesEuclid::getMaxDataAtt(double &m) const
{
  return getDouble("DataAtt_max", m);
}
  
/*----------------------------------------------------------------*/
bool AttributesEuclid::getAverageDataAtt(double &m) const
{
  return getDouble("DataAtt_ave", m);
}
  
