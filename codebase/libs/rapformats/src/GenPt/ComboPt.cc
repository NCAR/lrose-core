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
// ComboPt.cc
//
// Combination point - one 1D and one 2D point.
//
// Uses MultBuf to contain a 1D and 2D GenPt in a single buffer.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// April 2000
//////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/ComboPt.hh>
#include <rapformats/MultBufPart.hh>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;

// constructor

ComboPt::ComboPt()

{
  clear();
}

// destructor

ComboPt::~ComboPt()

{
}

// clear

void ComboPt::clear() {
  _errStr = "";
  _prodInfo = "";
  _1dPt.clear();
  _2dPt.clear();
  _buf.clearAll();
}

//////////////////////////////////////////////////////////////////
// Check we have a valid object.
// Use this after the set methods to check the object.
// On failure, use getErrStr() to see error

bool ComboPt::check() const
{

  _errStr = "ERROR - ComboPt::check()\n";
  bool ret = true;

  if (!_1dPt.check()) {
    _errStr += "ERROR - 1d point.\n";
    _errStr += _1dPt.getErrStr();
	ret = false;
  }

  if (!_2dPt.check()) {
    _errStr += "ERROR - 2d point.\n";
    _errStr += _2dPt.getErrStr();
	ret = false;
  }

  return ret;
}
///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void ComboPt::assemble()

{

  _buf.clearAll();
  _buf.setId(prodId);
  if (_prodInfo.size() > 0) {
    _buf.addPart(_infoPart, _prodInfo.size() + 1, _prodInfo.c_str());
  }
  if (_1dPt.check()) {
    _1dPt.assemble();
    _buf.addPart(_1dPart, _1dPt.getBufLen(), _1dPt.getBufPtr());
  }
  if (_2dPt.check()) {
    _2dPt.assemble();
    _buf.addPart(_2dPart, _2dPt.getBufLen(), _2dPt.getBufPtr());
  }
  _buf.assemble();

}

///////////////////////////////////////////////////////////
// disassemble() - sets the object values from a buffer
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int ComboPt::disassemble(const void *buf, int len)

{
  
  clear();
  _errStr = "ERROR - ComboPt::disassemble()\n";
  
  if (_buf.disassemble(buf, len)) {
    _errStr += "Cannot disassemble buffer into message.\n";
    return -1;
  }
  
  MultBufPart *part;
  
  // prodInfo
  
  if (_buf.partExists(_infoPart)) {
    part = _buf.getPartByType(_infoPart);
    _prodInfo = (char *) part->getBuf();
  }
  
  // 1d point

  if (_buf.partExists(_1dPart)) {
    part = _buf.getPartByType(_1dPart);
    if (_1dPt.disassemble(part->getBuf(), part->getLength())) {
      _errStr = "ERROR - ComboPt::disassemble()\n";
      _errStr += "  Cannot disassemble() 1d point\n";
      return -1;
    }
  }

  // 2d point

  if (_buf.partExists(_2dPart)) {
    part = _buf.getPartByType(_2dPart);
    if (_2dPt.disassemble(part->getBuf(), part->getLength())) {
      _errStr = "ERROR - ComboPt::disassemble()\n";
      _errStr += "  Cannot disassemble() 2d point\n";
      return -1;
    }
  }

  return 0;

}

// prints

void ComboPt::print(FILE *out) const

{
  fprintf(out, "  ==================================\n");
  fprintf(out, "  ComboPt - combination 1d/2d  point\n");
  fprintf(out, "  ==================================\n");
  fprintf(out, "  prodInfo: %s\n", _prodInfo.c_str());
  fprintf(out, "  1D point:\n");
  _1dPt.print(out);
  fprintf(out, "  2D point:\n");
  _2dPt.print(out);
}

void ComboPt::print(ostream &out) const

{
  out << "  ==================================" << endl;
  out << "  ComboPt - combination 1d/2d  point" << endl;
  out << "  ==================================" << endl;
  out << "  prodInfo: " << _prodInfo << endl;
  out << "  1D point:" << endl;
  _1dPt.print(out);
  out << "  2D point:" << endl;
  _2dPt.print(out);
}

