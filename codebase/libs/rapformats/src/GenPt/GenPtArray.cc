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
/////////////////////////////////////////////////////////////
// GenPtArray.cc
//
// C++ wrapper for generic point data.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2000
//////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/GenPtArray.hh>
#include <rapformats/MultBufPart.hh>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;

// constructor

GenPtArray::GenPtArray()

{
  clear();
}

// destructor

GenPtArray::~GenPtArray()

{
  clearPoints();
}

// clear

void GenPtArray::clear() {
  _errStr = "";
  _prodInfo = "";
  clearPoints();
}

void GenPtArray::clearPoints()

{
  for (size_t i = 0; i < _points.size(); i++) {
    delete _points[i];
  }
  _points.erase(_points.begin(), _points.end());
}

void GenPtArray::addPoint(const GenPt &point)

{
  GenPt *pt = new GenPt(point);
  _points.push_back(pt);
}


///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void GenPtArray::assemble()

{

  _buf.clearAll();
  _buf.setId(prodId);
  _buf.addPart(prodInfoPart, _prodInfo.size() + 1, _prodInfo.c_str());
  
  for (size_t i = 0; i < _points.size(); i++) {
    _points[i]->assemble();
    _buf.addPart(pointPart,
		 _points[i]->getBufLen(),
		 _points[i]->getBufPtr());
  }
  
  _buf.assemble();

}

///////////////////////////////////////////////////////////
// disassemble() - sets the object values from a buffer
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int GenPtArray::disassemble(const void *buf, int len)

{
  
  clear();
  _errStr = "ERROR - GenPtArray::disassemble()\n";
  
  if (_buf.disassemble(buf, len)) {
    _errStr += "Cannot disassemble buffer into message.\n";
    return -1;
  }
  
  MultBufPart *part;

  // prodInfo
  
  if (_buf.partExists(prodInfoPart)) {
    part = _buf.getPartByType(prodInfoPart);
    _prodInfo = (char *) part->getBuf();
  }

  // points

  int npoints = _buf.partExists(pointPart);
  
  for (int i = 0; i < npoints; i++) {
    part = _buf.getPartByType(pointPart, i);
    GenPt point;
    if (point.disassemble(part->getBuf(), part->getLength())) {
      _errStr = "ERROR - GenPtArray::disassemble()\n";
      TaStr::AddInt(_errStr,
		    "Cannot disassemble() for point number: ", i);
      return -1;
    }
    addPoint(point);
  }

  return 0;

}

// prints

void GenPtArray::print(FILE *out) const

{
  fprintf(out, "  ================================\n");
  fprintf(out, "  GenPtArray - generic point array\n");
  fprintf(out, "  ================================\n");
  fprintf(out, "  prodInfo: %s\n", _prodInfo.c_str());
  fprintf(out, "  Npoints: %d\n", (int) _points.size());
  for (size_t i = 0; i < _points.size(); i++) {
    fprintf(out, "---> Point: %d\n", (int) i);
    _points[i]->print(out);
  }
}

void GenPtArray::print(ostream &out) const

{
  out << "  ================================" << endl;
  out << "  GenPtArray - generic point array" << endl;
  out << "  ================================" << endl;
  out << "  prodInfo: " << _prodInfo << endl;
  out << "  Npoints: " << _points.size() << endl;
  for (size_t i = 0; i < _points.size(); i++) {
    out << "---> Point: " << i << endl;
    _points[i]->print(out);
  }
}

