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
 * @file CloudGap.hh 
 * @brief CloudGap The gap between two clouds along a beam
 *                 or between the radar and the 1st cloud
 * @class CloudGap 
 * @brief CloudGap The gap between two clouds along a beam
 *                 or between the radar and the 1st cloud
 *
 * Storage for information about range of indices along a beam
 * as you 'leave' the closer cloud and eventually 'enter' the farter cloud
 *
 * At the near boundary, the outside cloud indices are bigger than inside,
 * At the farther bounday, the outside cloud indices are smaller than cloud
 */

#ifndef CloudGap_H
#define CloudGap_H

#include "RayCloudEdge.hh"

class CloudEdge;
class Grid2d;

/*----------------------------------------------------------------*/
class CloudGap
{
public:
  /**
   * The gap between the radar and the first encountered cloud
   *
   * @param[in] e  The near edge of the first cloud
   */
  CloudGap(const RayCloudEdge &e);

  /**
   * The gap between two clouds
   *
   * @param[in] e0   farthest edge of the near cloud
   * @param[in] e1   nearest edge of the far cloud
   */
  CloudGap(const RayCloudEdge &e0, const RayCloudEdge &e1);

  /**
   * Destructor
   */
  virtual ~CloudGap();

  /**
   * @return clump value for the clump representing one of the two clouds
   * @param[in] isFar   True to return index for farther away
   *                     cloud, false for near cloud
   */
  inline double getValue(bool isFar) const
  {
    if (isFar)
    {
      return _far.getValue();
    }
    else
    {
      return _near.getValue();
    }
  }

  /**
   * Put 'in cloud' points for both clouds to the input grid, using the
   * value found in the objects, which is clump value.
   *
   * @param[in,out] data
   */
  void toGrid(Grid2d &data) const;

  /**
   * Put 'outside cloud' points to the input grid, for both clouds, using
   * the value found in the objects, which is clump value
   *
   * @param[in,out] data
   */
  void toOutsideGrid(Grid2d &data) const;

  /**
   * @return number of points in the gap between the near and far clouds
   */
  int nptBetween(void) const;
  
  /**
   * @return true if this is the gap from radar to the first cloud
   */
  inline bool isClosest(void) const
  {
    return _near.isAtRadar();
  }

  /**
   * @return beam (y) index
   */
  inline int getY(void) const {return _y;}

  /**
   * @return boundary x index for the cloud
   *
   * @param[in] isFar  true=Return smallest x index for far cloud
   *                   False=Return largest x index for near cloud
   */
  int getX(bool isFar) const;

  /**
   * Debug print to stdout
   */
  void print(void) const;

protected:
private:

  int _y;             /**< Y index (beam) */
  RayCloudEdge _near; /**< Near cloud near edge points */
  RayCloudEdge _far;  /**< Farther cloud near edge points */
};

#endif
 
