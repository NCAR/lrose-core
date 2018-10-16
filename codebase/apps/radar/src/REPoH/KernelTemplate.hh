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
 * @file KernelTemplate.hh 
 * @brief KernelTemplate
 * @class KernelTemplate
 * @brief KernelTemplate
 * 
 * the kernel building alg has a fixed order, the points look like this:
 *
 *    ^
 *    |
 *    x   y->
 *
 *
 *    28 24     25 29
 *    20 16     17 21
 *    12  6  1   7 13
 *    10  4  0   5 11
 *    8   2  X   3  9
 *    18 14     15 19
 *    26 22     23 27
 *
 * where X is the kernel starting point, and the order to add points
 * is as shown above 
 *
 * Array _off shows this order of evaluation
 *
 * _prev_needed is an index into previous points in kernel yes or no.
 * must be yes for at least one of those not equal to -1 
 * this is to prevent isolated points
 *
 *
 * when not 'moving in', the order is reversed in x, the points look like this:
 *
 *    ^
 *    |
 *    x   y->
 *
 *    26 22     23 27
 *    18 14     15 19
 *     8  2  X   3  9
 *    10  4  0   5 11
 *    12  6  1   7 13
 *    20 16     17 21
 *    28 24     25 29
 *
 * Array _offOut shows this order of evaluation
 *
 * _prev_needed is same for 'out', as it is symmetric, only inverted.
 * (neighbors are the same index).
 *
 */

#ifndef KernelTemplate_H
#define KernelTemplate_H

#include <euclid/Point.hh>

class Grid2d;
class KernelPoints;

/*----------------------------------------------------------------*/
class KernelTemplate
{
public:
  KernelTemplate(int nx, int ny, const Point &center, bool moving_in);
  ~KernelTemplate();

  void add_kernel_points(const Grid2d &mask, const int maxpt, KernelPoints &k);
  void add_kernel_outside_points(const Grid2d &mask, 
				 const int maxpt, KernelPoints &k);

protected:
private:
  bool _added[30];
  int _nx, _ny;
  Point _center;
  bool _moving_in;

  static const Point _off[30];
  static const Point _offOut[30];

  // static const int _x_off[30];
  // static const int _y_off[30];
  // static const int _x_off_out[30];
  // static const int _y_off_out[30];
  static const int _prev_needed[30][3];

  void _add_kernel_points(const Grid2d &mask, const int maxpt,
			  const Point *off, KernelPoints &k);
  void _add_kernel_outside_points(const Grid2d &mask,const int maxpt,
				  const int x0, const Point *off,
				  KernelPoints &k);
  bool _previous_ok(const int i) const;
};


#endif
 
