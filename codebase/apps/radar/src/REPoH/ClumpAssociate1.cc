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
ClumpAssociate1::ClumpAssociate1(const int c1_color)
{
  _c1_color = c1_color;
}

/*----------------------------------------------------------------*/
ClumpAssociate1::~ClumpAssociate1()
{
}

/*----------------------------------------------------------------*/
void ClumpAssociate1::print(void) const
{
  printf("Clump[%d] = [ ", _c1_color);
  for (int i=0; i<(int)_c2_color.size(); ++i)
    printf("%d ", _c2_color[i]);
  printf("]\n");
}

/*----------------------------------------------------------------*/
void ClumpAssociate1::add_color(const int c2_color)
{
  if (find(_c2_color.begin(), _c2_color.end(), c2_color) == _c2_color.end())
    _c2_color.push_back(c2_color);
}
