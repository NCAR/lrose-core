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
 * @file CloudEdge.hh 
 * @brief CloudEdge the 'edge' of a cloud.
 * @class CloudEdge
 * @brief CloudEdge the 'edge' of a cloud.
 */

#ifndef CloudEdge_H
#define CloudEdge_H

#include <string>
#include <toolsa/LogStream.hh>

/*----------------------------------------------------------------*/
class CloudEdge
{
public:
  /**
   * y = index to azimuth
   * x0,x1 = range of x just 'in' the cloud.
   * xout0, xout1 = range of x just 'outside' the cloud
   * v = clump color value
   * moving_in = true if at edge of cloud near radar, 
   *             false if at edge of cloud farther away from radar
   */
  CloudEdge(const int y, const int x0, const int x1, 
	    const int xout0, const int xout1, const double v,
	    const bool moving_in);
  ~CloudEdge();

  inline int get_y(void) const {return _y;}
  inline int get_x0(void) const {return _x0;}
  inline int get_x1(void) const {return _x1;}
  inline int get_xout0(void) const {return _xout0;}
  inline int get_xout1(void) const {return _xout1;}
  inline double get_color(void) const {return _v;}

  std::string sprint(void) const;
  void print(void) const;
  void log(const std::string &msg) const;

  inline bool too_close(const int min_x) const
  {
    return _x0 < min_x;
  }

  inline bool moving_in(void) const
  {
    return _movingIn;
  }

protected:
private:

  int _y;
  int _x0, _x1;
  int _xout0, _xout1;
  double _v;
  bool _movingIn;
};

#endif
 
