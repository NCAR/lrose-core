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
/////////////////////////////////////////////////////////////
// OutputGeometry.hh
//
// Specify the output geometry for the radar
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2001
//
/////////////////////////////////////////////////////////////

#ifndef OutputGeometry_HH

class OutputGeometry {

public:

  OutputGeometry();
  OutputGeometry(const int nx, const int ny,
                 const float dx, const float dy,
                 const float minx, const float miny,
                 const bool debug, const bool verbose);

  void setGeometry(const int nx, const int ny,
                   const float dx, const float dy,
                   const float minx, const float miny);

  int getNx()     const { return _nx; }
  int getNy()     const { return _ny; }
  float getDx()   const { return _dx; }
  float getDy()   const { return _dy; }
  float getMinx() const { return _minx; }
  float getMiny() const { return _miny; }

  bool isGeometrySet() const { return _isGeometrySet; }

  bool isEqual(const int nx, const int ny,
               const float dx, const float dy,
               const float minx, const float miny) const;

protected:

  int _nx, 
      _ny;

  float _dx,
        _dy,
        _minx,
        _miny;

  bool _isGeometrySet;

};

#endif
