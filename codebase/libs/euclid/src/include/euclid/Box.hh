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
 * @file Box.hh
 * @brief A 2 dimensional rectangle.
 * @class Box
 * @brief A 2 dimensional rectangle.
 */
# ifndef    BOX_H
# define    BOX_H

#include <string>

class Box
{
  friend class Line;
  friend class LineMask;

public:

  /**
   * Empty box
   */
  Box(void);

  /**
   * Box with specs
   *
   * @param[in] x0  Lower left
   * @param[in] y0  Lower left
   * @param[in] x1  Upper right
   * @param[in] y1  Upper right
   */
  Box(double x0, double y0, double x1, double y1);

  /**
   * Destructor
   */
  virtual ~Box(void);

  /**
   * @return true if box has not been specified
   */
  inline bool emptyBox(void) const
  {
    return !_ok;
  }

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   * @return string representation of the box
   */
  std::string sprint(void) const;

  /**
   * Get range of values
   * @param[out] min  Minimum value
   * @param[out] max  Maximum value
   * @param[in] is_x  True if range is for X, false if range is for Y
   */
  inline void getRange(double &min, double &max, bool is_x) const
  {
    if (is_x)
    {
      min = _minx;
      max = _maxx;
    }
    else
    {
      min = _miny;
      max = _maxy;
    }
  }

  /**
   * take average in x or y (center of box)
   * @param[in] is_x  True to take x average
   */
  inline double average(bool is_x) const
  {
    if (is_x)
      return (_minx + _maxx)/2.0;
    else
      return (_miny + _maxy)/2.0;
  }

  /**
   * Expand the box by maxdist in all directions.
   *
   * @param[in] maxdist
   * @param[in] no_truncate If true, no trunctaion, 
   *                        if false, truncate to keep inside grid
   * @param[in] nx  Grid upper limit
   * @param[in] ny  Grid upper limit
   */
  void expand(double maxdist, bool no_truncate, int nx, int ny);

  /**
   * expand the box so it contains the input box.
   * @param[in] b  Box that must be contained
   */
  void expand(const Box &b);

protected:

  double _minx;  /**< The lower left X */
  double _miny;  /**< The lower left Y */
  double _maxx;  /**< The upper right X */
  double _maxy;  /**< The upper right Y */

private:

  bool _ok;  /**< True if values are set */

  inline void _setBoxValues(double x0, double y0, double x1, double y1)
  {
    _minx = x0;
    _maxx = x1;
    _miny = y0;
    _maxy = y1;

  }

  void _truncateAtEdges(int nx, int ny);
};

# endif
