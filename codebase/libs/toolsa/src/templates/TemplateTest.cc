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
//////////////////////////////////////////////////////////////////////////////
//
// Test compile for templates
//
//////////////////////////////////////////////////////////////////////////////


#include <toolsa/TaArray.hh>
#include <toolsa/SignBit.hh>
#include <cmath>
#include <iostream>
using namespace std;

// test template for compile errors

void TemplateTest()

{

  // test TaArray

  TaArray<char> cc1_;
  char *cc1 = cc1_.alloc(10);
  cerr << "size of cc1: " << cc1_.size() << endl;
  cerr << "cc1: " << cc1 << endl;
  cc1_.free();

  TaArray<unsigned char> cc2_;
  unsigned char *cc2 = cc2_.alloc(10);
  cerr << "size of cc2: " << cc2_.size() << endl;
  cerr << "cc2: " << cc2 << endl;
  cc2_.free();

  TaArray<short> ss1_;
  short *ss1 = ss1_.alloc(10);
  cerr << "size of ss1: " << ss1_.size() << endl;
  cerr << "ss1: " << ss1 << endl;
  ss1_.free();

  TaArray<unsigned short> ss2_;
  unsigned short *ss2 = ss2_.alloc(10);
  cerr << "size of ss2: " << ss2_.size() << endl;
  cerr << "ss2: " << ss2 << endl;
  ss2_.free();

  TaArray<int> ii1_;
  int *ii1 = ii1_.alloc(10);
  cerr << "size of ii1: " << ii1_.size() << endl;
  cerr << "ii1: " << ii1 << endl;
  ii1_.free();

  TaArray<unsigned int> ii2_;
  unsigned int *ii2 = ii2_.alloc(10);
  cerr << "size of ii2: " << ii2_.size() << endl;
  cerr << "ii2: " << ii2 << endl;
  ii2_.free();

  TaArray<long> ll1_;
  long *ll1 = ll1_.alloc(10);
  cerr << "size of ll1: " << ll1_.size() << endl;
  cerr << "ll1: " << ll1 << endl;
  ll1_.free();

  TaArray<unsigned long> ll2_;
  unsigned long *ll2 = ll2_.alloc(10);
  cerr << "size of ll2: " << ll2_.size() << endl;
  cerr << "ll2: " << ll2 << endl;
  ll2_.free();

  TaArray<float> ff1_;
  float *ff1 = ff1_.alloc(10);
  cerr << "size of ff1: " << ff1_.size() << endl;
  cerr << "ff1: " << ff1 << endl;
  ff1_.free();

  TaArray<double> dd1_(10);
  double *dd1 = dd1_.buf();
  cerr << "size of dd1: " << dd1_.size() << endl;
  cerr << "dd1: " << dd1 << endl;
  dd1_.free();

  // Test SignBit

  int ii32 = -1000;
  cerr << "i32 isSet(" << ii32 << ") = " << SignBit::isSet(ii32) << endl;

  float xx32 = 250.0;
  cerr << "f32 isSet(" << xx32 << ") = " << SignBit::isSet(xx32) << endl;

  double xx64 = -0.0;
  cerr << "f64 isSet(" << xx64 << ") = " << SignBit::isSet(xx64) << endl;

}

