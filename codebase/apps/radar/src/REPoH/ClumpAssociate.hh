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
 * One clump color from one set of clump is associated with any number of 
 * clumps from the second set
 */

#ifndef ClumpAssociate_H
#define ClumpAssociate_H

#include "ClumpAssociate1.hh"
#include <rapmath/MathUserData.hh>
#include <vector>
#include <string>

class CloudGap;
class ClumpRegions;
class Grid2d;

/*----------------------------------------------------------------*/
class ClumpAssociate : public MathUserData
{
public:
  /**
   * @param[in] r  The clumps broken into invidual items
   * @param[in] clumps  The first set of clumps
   * @param[in] pidClumps  The second set, assumed larger
   *
   * for each clump in r, get the color and then for each point in the clump
   * see what color is in pidClumps, add all such colors to make the association
   */
  ClumpAssociate(const ClumpRegions &r, const Grid2d &clumps,
		 const Grid2d &pidClumps);


  virtual ~ClumpAssociate();

  #include <rapmath/MathUserDataVirtualMethods.hh>

  void print(void) const;

  /**
   * Get number of points penetration for a cloud gap

   * @param[in] g  Cloud gap
   * @param[in] pidClumps  = color coded clumping
   * @param[out] npt0  Number of points penetraion, near cloud
   * @param[out] npt1  Number of points penetraion, far cloud
   */
  void penetration(const CloudGap &g, const Grid2d &pidClumps,
		   int &npt0, int &npt1);

  /**
   * @return strings describing the assocations for the penetration
   */
  std::vector<std::string> get_penetration_report(void) const;

protected:
private:

  /**
   * The clump associations for each of the first set of clumps
   */
  std::vector<ClumpAssociate1> _elem;


  /**
   * Workspace variables
   */
  int _x0;  /**< far cloud first X value */
  int _x1;  /**< Near cloud last X value */
  int _y;   /**< Beam index */
  int _color0;  /**< clump value, far cloud */
  int _color1;  /**< clump value, near cloud */
  std::vector<int> _pid_colors0; /**< PID clump colors involved in far cloud */
  std::vector<int> _pid_colors1; /**< PID clump colors involved in near cloud*/


  const ClumpAssociate1 *_get_matching(const int color) const;
};

#endif
 
