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
// ComboPt.hh
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

#ifndef _ComboPt_hh
#define _ComboPt_hh


#include <rapformats/GenPt.hh>
#include <rapformats/MultBuf.hh>
using namespace std;

class ComboPt {

public:

  static const int prodId = 61;

  typedef enum {
    _infoPart = 1,
    _1dPart = 2,
    _2dPart = 3
  } part_type;
  
  ComboPt();
  ~ComboPt();

  //////////////////////////////////////////////
  // Check we have a valid object.
  // Use this after the set methods to check the object.
  // On failure, use getErrStr() to see error

  bool check() const;

  // set methods
  
  void clear();
  void setProdInfo(const string &prodInfo) { _prodInfo = prodInfo; }
  void set1DPoint(const GenPt &point) { _1dPt = point; }
  void set2DPoint(const GenPt &point) { _2dPt = point; }
  
  // get methods

  const string &getProdInfo() const { return (_prodInfo); }
  
  const GenPt &get1DPoint() { return (_1dPt); }
  const GenPt &get2DPoint() { return (_2dPt); }

  void *getBufPtr() const { return _buf.assembledBuf(); }
  int getBufLen() const { return _buf.lengthAssembled(); }

  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();
  
  // disassemble() - sets the object values from a buffer
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  // error string
  
  const string &getErrStr() const { return (_errStr); }
  
  // print
  
  void print(FILE *out) const;
  void print(ostream &out) const;

protected:

  mutable string _errStr;
  string _prodInfo;
  GenPt _1dPt;
  GenPt _2dPt;
  MultBuf _buf;

private:

};


#endif
