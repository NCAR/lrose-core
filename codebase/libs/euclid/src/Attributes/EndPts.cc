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
#include <cstdio>
#include <euclid/EndPts.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>

/*----------------------------------------------------------------*/
Endpts::Endpts()
{
  _index0 = -1;
  _endpt0 = -1;
  _index1 = -1;
  _endpt1 = -1;
}

/*----------------------------------------------------------------*/
Endpts::Endpts(int i0, int e0, int i1, int e1)
{
  _index0 = i0;
  _endpt0 = e0;
  _index1 = i1;
  _endpt1 = e1;
}


/*----------------------------------------------------------------*/
Endpts::~Endpts()
{
}

/*----------------------------------------------------------------*/
bool Endpts::operator==(const Endpts &e) const
{
  return (_index0 == e._index0 && _endpt0 == e._endpt0 &&
	  _index1 == e._index1 && _endpt1 == e._endpt1);
}

/*----------------------------------------------------------------*/
std::string Endpts::writeXml(void) const
{
  string ret = TaXml::writeStartTag("EndPts", 0);
  ret += TaXml::writeInt("Index0", 0, _index0);
  ret += TaXml::writeInt("EndPt0", 0, _endpt0);
  ret += TaXml::writeInt("Index1", 0, _index1);
  ret += TaXml::writeInt("EndPt1", 0, _endpt1);
  ret += TaXml::writeEndTag("EndPts", 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool Endpts::readXml(const std::string &xml)
{
  *this = Endpts();

  string buf;
  if (TaXml::readString(xml, "EndPts", buf))
  {
    LOG(ERROR) << "reading tag EndPts";
    return false;
  }
  if (TaXml::readInt(buf, "Index0", _index0))
  {
    LOG(ERROR) << "reading tag Index0";
    return false;
  }
  if (TaXml::readInt(buf, "EndPt0", _endpt0))
  {
    LOG(ERROR) << "reading tag EndPt0";
    return false;
  }

  if (TaXml::readInt(buf, "Index1", _index1))
  {
    LOG(ERROR) << "reading tag Index1";
    return false;
  }
  if (TaXml::readInt(buf, "EndPt1", _endpt1))
  {
    LOG(ERROR) << "reading tag EndPt1";
    return false;
  }
  return true;
}
  

/*----------------------------------------------------------------*/
void Endpts::get(int which, int &index, int &endpt) const
{
  if (which == 0)
  {
    index = _index0;
    endpt = _endpt0;
  }
  else if (which == 1)
  {
    index = _index1;
    endpt = _endpt1;
  }
  else
  {
    index = -1;
    endpt = 0;
  }
}

/*----------------------------------------------------------------*/
void Endpts::merge(const Endpts &i)
{
  if (i._index0 < _index0)
  {

    _index0 = i._index0;
    _endpt0 = i._endpt0;
  }
  else if (i._index0 == _index0)
  {
    if (i._endpt0 < _endpt0)
      _endpt0 = i._endpt0;
  }

  if (i._index1 > _index1)
  {
    _index1 = i._index1;
    _endpt1 = i._endpt1;
  }
  else if (i._index1 == _index1)
  {
    if (i._endpt1 > _endpt1)
      _endpt1 = i._endpt1;
  }
}

/*----------------------------------------------------------------*/
// return the new endpoint index from 0'th of this to 1th of l1
Endpts Endpts::average(const Endpts &i) const
{
  int i1, e1;
    
  i.get(1, i1, e1);
    
  Endpts ret(_index0, _endpt0, i1, e1);
  return ret;
}

/*----------------------------------------------------------------*/
void Endpts::print(FILE *fp) const
{
  fprintf(fp, "%s", sprint().c_str());
}

/*----------------------------------------------------------------*/
void Endpts::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
std::string Endpts::sprint(void) const
{
  char buf[1000];
  sprintf(buf, "0:(%d,%d) 1:(%d,%d)", _index0, _endpt0, _index1,
	  _endpt1);
  std::string ret = buf;
  return ret;
}

