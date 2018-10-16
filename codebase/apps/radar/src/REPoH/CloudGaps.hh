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
 * @file CloudGaps.hh 
 * @brief CloudGaps gaps between clouds (where humidity can be estimated)
 *                  for an entire scan
 * @class CloudGaps
 * @brief CloudGaps gaps between clouds (where humidity can be estimated)
 *                  for an entire scan
 * 
 */

#ifndef CloudGaps_H
#define CloudGaps_H

#include "CloudGap.hh"
#include <rapmath/MathUserData.hh>
#include <vector>

class ClumpAssociate;
class Grid2d;

/*----------------------------------------------------------------*/
class CloudGaps : public MathUserData
{
public:

  CloudGaps();
  ~CloudGaps();

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * Add all gaps for a beam
   *
   * @param[in] y  index to beam
   * @param[in] nx  Number of gates
   * @param[in] clumps  Clump data
   * @param[in] depth  Maximum number of gates to penetrate the cloud
   */
  void addGaps(int y, int nx, const Grid2d &clumps, int depth);

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * @return a grid with cloud edge points in it
   *
   * @param[in] nx  Grid dimension
   * @param[in] ny  Grid dimension
   */
  Grid2d edge(int nx, int ny) const;

  /**
   * @return a grid with 'just outside a cloud' points in it
   *
   * @param[in] nx  Grid dimension
   * @param[in] ny  Grid dimension
   */
  Grid2d outside(int nx, int ny) const;

  /**
   * @return Number of CloudGap objects
   */
  inline int num(void) const
  {
    return (int)_gap.size();
  }

  /**
   * @return reference to the 'ith' CloudGap object
   */
  inline const CloudGap &ith_cloudgap(int i) const
  {
    return _gap[i];
  }

  /**
   * Filter out all CloudGaps that are too close together
   *
   * @param[in] minGridpt  Minimum number of points seperation
   */
  void filter(int minGridpt);

  /**
   * Filter out all CloudGaps that penetrate too far into a storm
   *
   * @param[in] maxGridpt  Maximum number of points penetration
   */
  void filter(const Grid2d &pid_clumps, ClumpAssociate &ca, int maxGridpt);

protected:
private:

  /**
   * All the gaps
   */
  std::vector<CloudGap> _gap;
};

#endif
 
