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
/**
 * @file Fuzzy.cc
 */
#include <FiltAlg/Fuzzy.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Fuzzy::Fuzzy() : FuzzyF()
{
  _ok = false;
}

//------------------------------------------------------------------
Fuzzy::Fuzzy(const FiltAlgParams &P, const int index) : FuzzyF()
{
  _ok = true;
  switch (index)
  {
  case 0:
    _build(P.fuzzy0_n, P._fuzzy0);
    break;
  case 1:
    _build(P.fuzzy1_n, P._fuzzy1);
    break;
  case 2:
    _build(P.fuzzy2_n, P._fuzzy2);
    break;
  case 3:
    _build(P.fuzzy3_n, P._fuzzy3);
    break;
  case 4:
    _build(P.fuzzy4_n, P._fuzzy4);
    break;
  case 5:
    _build(P.fuzzy5_n, P._fuzzy5);
    break;
  case 6:
    _build(P.fuzzy6_n, P._fuzzy6);
    break;
  case 7:
    _build(P.fuzzy7_n, P._fuzzy7);
    break;
  case 8:
    _build(P.fuzzy8_n, P._fuzzy8);
    break;
  case 9:
    _build(P.fuzzy9_n, P._fuzzy9);
    break;
  case 10:
    _build(P.fuzzy10_n, P._fuzzy10);
    break;
  case 11:
    _build(P.fuzzy11_n, P._fuzzy11);
    break;
  case 12:
    _build(P.fuzzy12_n, P._fuzzy12);
    break;
  case 13:
    _build(P.fuzzy13_n, P._fuzzy13);
    break;
  case 14:
    _build(P.fuzzy14_n, P._fuzzy14);
    break;
  default:
    LOG(ERROR) << "index " << index << " out of range 0 to 11";
    _ok = false;
  }
}

//------------------------------------------------------------------
Fuzzy::~Fuzzy()
{
}

//------------------------------------------------------------------
void Fuzzy::_build(const int n, const FiltAlgParams::fuzzy_t *ff)
{
  vector<pair<double,double> > f;
  for (int i=0; i<n; ++i)
  {
    pair<double,double> pdd(ff[i].x, ff[i].y);
    f.push_back(pdd);
  }
  *((FuzzyF *)this) = FuzzyF(f);
}
