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
 * @file ClumpRegion.hh
 * @brief 
 * @class ClumpRegion
 * @brief 
 */

# ifndef    ClumpRegion_HH
# define    ClumpRegion_HH

#include <euclid/Grid2dClump.hh> // needed for region_t definition below
#include <vector>

class ClumpRegion
{
public:

  /**
   * Constructor
   * @param[in] r  Region points
   */
  inline ClumpRegion(const clump::Region_t &r)
  {
    for (size_t i=0; i<r.size(); ++i)
    {
      _pt.push_back(r[i]);
    }
  }

  inline ~ClumpRegion(void) {}
  inline size_t size(void) const {return _pt.size();}
  inline int x(int i) const {return _pt[i].first;}
  inline int y(int i) const {return _pt[i].second;}

  /**
   * Set all points in input grid for this clumpregion to missing
   * @param[in,out] data
   */
  void setMissing(Grid2d &data) const;

  /**
   * Set all points in input grid for this clumpregion to a value
   * @param[in] value
   * @param[in,out] data
   */
  void setToValue(double value, Grid2d &data) const;
  
private:

  /**
   * Some points
   */
  std::vector<std::pair<int,int> > _pt;
};

#endif
