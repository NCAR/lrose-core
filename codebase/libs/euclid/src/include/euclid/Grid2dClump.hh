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
 * @file Grid2dClump.hh
 * @brief algorithms to break grids into disjoint subsets.
 * @class Grid2dClump
 * @brief algorithms to break grids into disjoint subsets.
 *
 * Grid2dClump produces connected subsets of points called 'regions'.
 */

# ifndef    GRID2D_CLUMP_H
# define    GRID2D_CLUMP_H

#include <vector>
#include <euclid/Grid2d.hh>

#define NO_MARKER 0.0
#define THISCLUMP_NOTDONE 10.0
#define THISCLUMP_DONE 15.0
#define PROCESSED 20.0

namespace clump
{
  /**
   * Each point in a region is a grid location <x,y>
   */
  typedef std::vector<std::pair<int,int> >                  Region_t;
  typedef std::vector<std::pair<int,int> >::iterator        Region_iter_t;
  typedef std::vector<std::pair<int,int> >::const_iterator  Region_citer_t;
}

class PointList;

//------------------------------------------------------------------
class Grid2dClump : public Grid2d
{      
public:

  /**
   * @param[in] g  grid to do clumping on.
   *
   * After construction ready to do the clumping algorithm
   */
  Grid2dClump(const Grid2d &g);

  /**
   * @param[in] g  Grid to do clumping on.
   * @param[in] bad  A value to ignore in grid.
   *
   * After construction ready to do the clumping algorithm
   */
  Grid2dClump(const Grid2d &g, const double bad);

  /**
   * Destructor
   */
  virtual ~Grid2dClump(void);

  /**
   * Debug print
   */
  void print(void) const;

  /** 
   * build all the disjoint regions you can.
   * @return the vector of region point lists.
   */
  std::vector<clump::Region_t> buildRegions(void);

  /**
   * build all the disjoint regions you can.
   * @return the vector of PointList objects
   */
  std::vector<PointList> buildRegionPointlists(void);


  /** 
   * build all the disjoint regions you can...a fully recursive algorithm.
   * @return the vector of region point lists.
   *
   * @note this method bogged down with large clumps (stack overflowed) which
   * led to the buildRegions() method as an alternative
   */
  std::vector<clump::Region_t> buildRegionsRecursive(void);

protected:
private:

  Grid2d _iwork;      /**< storage for checked/unchecked points. */
  clump::Region_t _n; /**< region points. */
  int _nx;            /**< grid dimension */
  int _ny;            /**< grid dimension */

  void _buildRegion(int ix, int iy);
  void _buildRegionRecursive(int ix, int iy);
  void _growRecursive(int x, int y);
  std::pair<int,int> _growNonrecursive(int x, int y, bool &done);
  std::pair<int, int> _findNondone(bool &done) const;
  void _buildN(void);
  bool _growOk(int ix, int iy, int x, int y) const;
  bool _growOkNonrecursive(int ix, int iy, int x, int y) const;
};

# endif 
