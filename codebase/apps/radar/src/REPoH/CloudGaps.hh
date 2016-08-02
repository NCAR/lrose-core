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
 * @class CloudGaps
 * @brief CloudGaps gaps between clouds (where humidity can be estimated)
 * 
 */

#ifndef CloudGaps_H
#define CloudGaps_H

#include <euclid/Grid2d.hh>
#include <vector>

class CloudGap;
class ClumpAssociate;

/*----------------------------------------------------------------*/
class CloudGaps
{
public:
  CloudGaps();
  ~CloudGaps();

  void print(void) const;

  /**
   * Return a grid with cloud edge points in it
   */
  Grid2d edge(const int nx, const int ny) const;

  /**
   * Return a grid with 'just outside a cloud' points in it
   */
  Grid2d outside(const int nx, const int ny) const;

  /**
   * Add all gaps for a beam
   * y=index to beam
   */
  void add_gaps(const int y, const int nx, const Grid2d &clumps,
		const int depth);

  /**
   * Return # of CloudGap objects
   */
  inline int num(void) const
  {
    return (int)_gap.size();
  }

  /**
   * Return ref. to the 'ith' CloudGap object
   */
  inline const CloudGap &ith_cloudgap(const int i) const
  {
    return _gap[i];
  }

  /**
   * Filter out all CloudGaps that are too close together
   * min_gridpt = minimum # of points seperation
   */
  void filter(const int min_gridpt);

  /**
   * Filter out all CloudGaps that penetrate too far into a storm
   * max_gridpt = maximum # of points penetration
   */
  void filter(const Grid2d &pid_clumps, ClumpAssociate &ca,
	      const int max_gridpt);

protected:
private:

  std::vector<CloudGap> _gap;

  void _filter(std::vector<CloudGap> &pts, const int min_gridpt);
};

#endif
 
