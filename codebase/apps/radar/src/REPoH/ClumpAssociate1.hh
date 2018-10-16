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
 * @file ClumpAssociate1.hh 
 * @brief ClumpAssociate1 Associate a clump with a another set of clumps
 * @class ClumpAssociate1
 * @brief ClumpAssociate1 Associate a clump with a another set of clumps
 *
 * The first clump has color  _c1Color
 * The other clumps is the colors vector _c2Color
 */

#ifndef ClumpAssociate1_H
#define ClumpAssociate1_H
#include <vector>

/*----------------------------------------------------------------*/
class ClumpAssociate1
{
public:
  ClumpAssociate1(const int c1Color);
  ~ClumpAssociate1();

  void print(void) const;

  void addColor(const int c2Color);
  inline int numColor(void) const {return (int)_c2Color.size();}
  inline int ithColor(const int i) const {return _c2Color[i];}
  inline int getColor(void) const {return _c1Color;}

protected:
private:

  int _c1Color;
  std::vector<int> _c2Color;
  
};

#endif
 
