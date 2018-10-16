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
 * @file ClumpAssociate1.cc
 */
#include "ClumpAssociate1.hh"

#include <cstdio>
#include <algorithm>

/*----------------------------------------------------------------*/
ClumpAssociate1::ClumpAssociate1(const int c1Color)
{
  _c1Color = c1Color;
}

/*----------------------------------------------------------------*/
ClumpAssociate1::~ClumpAssociate1()
{
}

/*----------------------------------------------------------------*/
void ClumpAssociate1::print(void) const
{
  printf("Clump[%d] = [ ", _c1Color);
  for (size_t i=0; i<_c2Color.size(); ++i)
  {
    printf("%d ", _c2Color[i]);
  }
  printf("]\n");
}

/*----------------------------------------------------------------*/
void ClumpAssociate1::addColor(const int c2Color)
{
  if (find(_c2Color.begin(), _c2Color.end(), c2Color) == _c2Color.end())
    _c2Color.push_back(c2Color);
}
