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
 * @brief CloudGap the gap between two clouds along a beam
 * @class CloudGap 
 * @brief CloudGap the gap between two clouds along a beam
 * 
 */

#ifndef CloudGap_H
#define CloudGap_H

class CloudEdge;
class Grid2d;

/*----------------------------------------------------------------*/
class CloudGap
{
public:
  /**
   * The gap between the radar and the first encountered cloud
   */
  CloudGap(const CloudEdge &e);

  /**
   * The gap between two clouds
   * e0 = closest clouds exit points
   * e1 = further clouds entry points.
   */
  CloudGap(const CloudEdge &e0, const CloudEdge &e1);

  virtual ~CloudGap();

  /**
   * Return this clouds color index value
   * is_far = true for the further out cloud
   */
  inline double get_color(const bool is_far) const
  {
    if (is_far)
      return _v1;
    else
      return _v0;
  }

  /**
   * Put cloud points to the input grid.
   */
  void to_grid(Grid2d &data) const;

  /**
   * Put cloud 'just outside' points to the input grid.
   */
  void to_outside_grid(Grid2d &data) const;

  /**
   * @return # of points in the gap
   */
  int npt_between(void) const;

  
  inline bool is_closest(void) const
  {
    return _xout0[0] == -1 &&_xout0[1] == -1;
  }

#ifdef NOTDEF
  void get_close_kernel_info(int &x,  double &v) const;

  void get_far_kernel_info(int &x,  double &v) const;


#endif

  inline int get_y(void) const {return _y;}

  /**
   * return index to x where first touch the storm (near or far)
   */
  int get_x(const bool is_far) const;

  void print(void) const;

protected:
private:

  /**
   * y index value for the gap
   */
  int _y;

  /**
   * run of points in the near cloud
   */
  int _x0[2]; 

  /**
   * Run of points just outside the  near cloud (farther from radar)
   */
  int _xout0[2];

  /** 
   * Value of the clump (color) for near cloud.
   */
  double _v0;

  /**
   * Run of points in the farther cloud
   */
  int _x1[2];

  /**
   * Run of points just outside the farther cloud (closer to radar)
   */
  int _xout1[2];

  /** 
   * Value of the clump (color) for far cloud.
   */
  double _v1;
};

#endif
 
