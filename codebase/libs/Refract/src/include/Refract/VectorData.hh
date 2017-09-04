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
 *
 * @file VectorData.hh
 *
 * @class VectorData
 *
 * slope of range, a one dimensional grid, one slope per beam
 *  
 */

#ifndef VectorData_H
#define VectorData_H

#include <vector>
#include <cmath>

/** 
 * @class VectorData
 */

class VectorData
{
  
public:

  inline VectorData(void) : _num(0) {}

  inline VectorData(int num, double init_value) : _num(num)
  {
    _data.reserve(num);
    for (int i=0; i<num; ++i)
    {
      _data.push_back(init_value);
    }
  }

  inline int slopeFloor(int index) const
  {
    return (((int)(floor(_data[index] + 0.5)) + 360000) % 360);
  }

  inline ~VectorData(void) {}

  /**
   * operator[], no bounds checking
   * @return reference to data at an index
   * @param[in] i
   */
  inline double &operator[](size_t i) {return _data[i];}

  /**
   * operator[], no bounds checking
   * @return reference to data at an index
   * @param[in] i
   */
  inline const double &operator[](size_t i) const {return _data[i];}

  inline void shiftDown(int i0, int i1)
  {
    for (int i=i0; i<i1; ++i)
    {
      _data[i] = _data[i+1];
    }
    _data[i1] = 0.0;
  }

  inline void setAllZero(void)
  {
    for (int i=0; i<_num; ++i)
    {
      _data[i] = 0.0;
    }
  }
  
private:
  std::vector<double> _data;
  int _num;
};

#endif
