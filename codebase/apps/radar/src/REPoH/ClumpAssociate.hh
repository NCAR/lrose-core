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
 * @file ClumpAssociate.hh 
 * @brief ClumpAssociate Associate clumps generated independently
 * @class ClumpAssociate
 * @brief ClumpAssociate Associate clumps generated independently
 * 
 */

#ifndef ClumpAssociate_H
#define ClumpAssociate_H

#include "ClumpAssociate1.hh"
#include <euclid/Grid2dClump.hh> // needed for Region_t typedef
class CloudGap;

/*----------------------------------------------------------------*/
class ClumpAssociate
{
public:
  ClumpAssociate(const std::vector<clump::Region_t> &r, const Grid2d &clumps,
		 const Grid2d &pid_clumps);
  ~ClumpAssociate();

  void print(void) const;

  void penetration(const CloudGap &g, const Grid2d &pid_clumps,
		   int &npt0, int &npt1);

  std::vector<std::string> get_penetration_report(void) const;

protected:
private:

  std::vector<ClumpAssociate1> _elem;


  int _x0;
  int _x1;
  int _y;
  int _color0;
  int _color1;
  std::vector<int> _pid_colors0;
  std::vector<int> _pid_colors1;


  const ClumpAssociate1 *_get_matching(const int color) const;
};

#endif
 
