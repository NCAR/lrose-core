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

/************************************************************************
 * CircularTemplate.hh: class implementing a circular template to be
 *                      applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CircularTemplate_HH
#define CircularTemplate_HH

#include <vector>

#include <cstdio>

#include <euclid/GridOffset.hh>
#include <euclid/GridPoint.hh>
#include <euclid/GridTemplate.hh>


class CircularTemplate : public GridTemplate
{
 public:

  // Constructor

  CircularTemplate(const double radius = 0.0); // Radius in number of grid squares
  
  // Destructor

  ~CircularTemplate(void);
  
  // Print the offset list to the given stream.  This is used for debugging.

  void printOffsetList(FILE *stream);
  
  // Access methods

  double getRadius(void) const
  {
    return _radius;
  }
  
  void setRadius(const double radius);
  
 private:

  // The circle radius in number of grid squares

  double _radius;
  
};


#endif
