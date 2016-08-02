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
#include <toolsa/Attributes.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
#include <cstdio>
#include <cmath>
using std::map;
using std::string;
using std::vector;
using std::pair;

/*----------------------------------------------------------------*/
Attributes::Attributes(void)
{
}

/*----------------------------------------------------------------*/
Attributes::~Attributes()
{
}

/*----------------------------------------------------------------*/
Attributes::Attributes(const vector<Attributes> &v,
		       const std::vector<std::string> &maxNames)
{
  map<string, pair<int,int> > intSums;
  map<string, pair<int,double> > doubleSums;

  for (size_t vi=0; vi<v.size(); ++vi)
  {
    map<string, int>::const_iterator i;
    for (i=v[vi]._di.begin(); i!= v[vi]._di.end(); ++i)
    {
      string name = i->first;
      map<std::string, pair<int,int> >::iterator ii = intSums.find(name);
      if (ii == intSums.end())
      {
	intSums[name] = pair<int,int>(1, i->second);
      }
      else
      {
	pair<int,int> p = intSums[name];
	if (find(maxNames.begin(), maxNames.end(), name) == maxNames.end())
	{
	  p.first += 1;
	  p.second += i->second;
	  intSums[name] = p;
	}
	else
	{
	  if (i->second > p.second)
	  {
	    p.second = i->second;
	    intSums[name] = p;
	  }
	}
      }
    }

    map<string, double>::const_iterator a;
    for (a=v[vi]._da.begin(); a!= v[vi]._da.end(); ++a)
    {
      string name = a->first;
      map<std::string, pair<int,double> >::const_iterator jj =
	doubleSums.find(name);
      if (jj == doubleSums.end())
      {
	doubleSums[name] = pair<int,double>(1, a->second);
      }
      else
      {
	pair<int,double> p = doubleSums[name];
	if (find(maxNames.begin(), maxNames.end(), name) == maxNames.end())
	{
	  p.first += 1;
	  p.second += a->second;
	  doubleSums[name] = p;
	}
	else
	{
	  if (a->second > p.second)
	  {
	    p.second = a->second;
	    doubleSums[name] = p;
	  }
	}
      }
    }
  }

  map<string, pair<int,int> >::iterator si;

  for (si=intSums.begin(); si!= intSums.end(); ++si)
  {
    if (find(maxNames.begin(), maxNames.end(), si->first) == maxNames.end())
    {
      if (si->second.first == 0)
      {
	LOG(ERROR) << "Dividing by zero, name=" << si->first;
      }
      else
      {
	int v = si->second.second/si->second.first;
	_di[si->first] = v;
      }
    }
    else
    {
      _di[si->first] = si->second.second;
    }
  }      

  map<string, pair<int,double> >::iterator sa;

  for (sa=doubleSums.begin(); sa!= doubleSums.end(); ++sa)
  {
    if (find(maxNames.begin(), maxNames.end(), sa->first) == maxNames.end())
    {
      if (sa->second.first == 0)
      {
	LOG(ERROR) << "Dividing by zero, name=" << sa->first;
      }
      else
      {
	double v = sa->second.second/(double)sa->second.first;
	_da[sa->first] = v;
      }
    }
    else
    {
      _da[sa->first] = sa->second.second;
    }
  }      
}  

/*----------------------------------------------------------------*/
Attributes::Attributes(const Attributes &a)
{
  _da = a._da;
  _di = a._di;
}

/*----------------------------------------------------------------*/
Attributes & Attributes::operator=(const Attributes &a)
{
  if (&a == this)
  {
    return *this;
  }

  _da = a._da;
  _di = a._di;
  return *this;
}

/*----------------------------------------------------------------*/
bool Attributes::operator==(const Attributes &a) const
{
  return _da == a._da && _di == a._di;
}

/*----------------------------------------------------------------*/
string Attributes::writeAttXml(const std::string &tag) const
{
  string ret = TaXml::writeStartTag(tag, 0);

  std::map<std::string, double>::const_iterator a;
  for (a=_da.begin(); a!= _da.end(); ++a)
  {
    ret += TaXml::writeStartTag("DoubleAtt", 0);
    ret += TaXml::writeString("Key", 0, a->first);
    ret += TaXml::writeDouble("Value", 0, a->second, "%.7lf");
    ret += TaXml::writeEndTag("DoubleAtt", 0);
  }

  std::map<std::string, int>::const_iterator i;
  for (i=_di.begin(); i!= _di.end(); ++i)
  {
    ret += TaXml::writeStartTag("IntAtt", 0);
    ret += TaXml::writeString("Key", 0, i->first);
    ret += TaXml::writeInt("Value", 0, i->second);
    ret += TaXml::writeEndTag("IntAtt", 0);
  }
  ret += TaXml::writeEndTag(tag, 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool Attributes::readAttXml(const std::string &xml, const std::string &tag)
{
  _da.clear();
  _di.clear();

  string buf;
  if (TaXml::readString(xml, tag, buf))
  {
    LOG(ERROR) << "Parsing tag " << tag << " in xml";
    return false;
  }

  vector<string> atts;
  if (TaXml::readStringArray(buf, "DoubleAtt", atts))
  {
    // assume no double attributes
  }
  else
  {
    for (size_t i=0; i<atts.size(); ++i)
    {
      string key;
      double value;
      if (TaXml::readString(atts[i], "Key", key))
      {
	LOG(ERROR) << "Parsing Key";
	return false;
      }
      if (TaXml::readDouble(atts[i], "Value", value))
      {
	LOG(ERROR) << "Parsing Value";
	return false;
      }
      _da[key] = value;
    }
  }

  if (TaXml::readStringArray(buf, "IntAtt", atts))
  {
    // assume no int attributes
  }
  else
  {
    for (size_t i=0; i<atts.size(); ++i)
    {
      string key;
      int value;
      if (TaXml::readString(atts[i], "Key", key))
      {
	LOG(ERROR) << "Parsing Key";
	return false;
      }
      if (TaXml::readInt(atts[i], "Value", value))
      {
	LOG(ERROR) << "Parsing Value";
      }
      _di[key] = value;
    }
  }
  return true;
}


/*----------------------------------------------------------------*/
void Attributes::attributeUnion(const Attributes &a)
{
  // overwrite only if not already in place
  map<std::string, double>::const_iterator i;
  map<std::string, double>::iterator li;
  for (i=a._da.begin(); i!=a._da.end(); ++i)
  {
    li = _da.find(i->first);
    if (li == _da.end())
    {
      _da[i->first] = i->second;
    }
  }

  map<std::string, int>::const_iterator j;
  map<std::string, int>::iterator lj;
  for (j=a._di.begin(); j!=a._di.end(); ++j)
  {
    lj = _di.find(j->first);
    if (lj == _di.end())
    {
      _di[j->first] = j->second;
    }
  }
}

/*----------------------------------------------------------------*/
void Attributes::addDouble(const std::string &name, const double v)
{
  _da[name] = v;
}

/*----------------------------------------------------------------*/
void Attributes::addInt(const std::string &name, const int i)
{
  _di[name] = i;
}

/*----------------------------------------------------------------*/
bool Attributes::getDouble(const std::string &name, double &v) const
{
  map<std::string, double>::const_iterator i = _da.find(name);
  if (i != _da.end())
  {
    v = i->second;
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
bool Attributes::getInt(const std::string &name, int &iv) const
{
  map<std::string, int>::const_iterator i = _di.find(name);
  if (i != _di.end())
  {
    iv = i->second;
    return true;
  }
  else
  {
    return false;
  }
}

/*----------------------------------------------------------------*/
void Attributes::removeDouble(const std::string &name)
{
  map<std::string, double>::iterator i = _da.find(name);
  if (i != _da.end())
  {
    _da.erase(i);
  }
}
/*----------------------------------------------------------------*/
void Attributes::removeInt(const std::string &name)
{
  map<std::string, int>::iterator i = _di.find(name);
  if (i != _di.end())
  {
    _di.erase(i);
  }
}

/*----------------------------------------------------------------*/
std::vector<int> Attributes::getAllValues(double scale) const
{
  vector<int> ret;
  map<std::string, int>::const_iterator i;
  for (i=_di.begin(); i!=_di.end(); ++i)
  {
    ret.push_back(i->second);
  }
  map<std::string, double>::const_iterator j;
  for (j=_da.begin(); j!=_da.end(); ++j)
  {
    ret.push_back(static_cast<int>(j->second*scale));
  }
  return ret;
}

/*----------------------------------------------------------------*/
void Attributes::printAtt(void) const
{
  printAtt(stdout);
}

/*----------------------------------------------------------------*/
void Attributes::printAtt(FILE *fp) const
{
  map<std::string, int>::const_iterator i;
  for (i=_di.begin(); i!=_di.end(); ++i)
  {
    fprintf(fp, "%s=%d ", i->first.c_str(), i->second);
  }
  map<std::string, double>::const_iterator j;
  for (j=_da.begin(); j!=_da.end(); ++j)
  {
    fprintf(fp, "%s=%lf ", j->first.c_str(), j->second);
  }
}

/*----------------------------------------------------------------*/
std::string Attributes::sprintAtt(void) const
{
  std::string ret = "";
  char buf[1000];
  map<std::string, int>::const_iterator i;
  for (i=_di.begin(); i!=_di.end(); ++i)
  {
    sprintf(buf, "%s=%d ", i->first.c_str(), i->second);
    ret += buf;
  }
  map<std::string, double>::const_iterator j;
  for (j=_da.begin(); j!=_da.end(); ++j)
  {
    sprintf(buf, "%s=%lf ", j->first.c_str(), j->second);
    ret += buf;
  }
  return ret;
}

