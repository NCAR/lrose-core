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
 * RectangularTemplate.hh: class implementing a Rectangular template to be
 *                      applied on gridded data.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2007
 *
 * Dan Megenhardt
 *
 ************************************************************************/

#ifndef RectangularTemplate_HH
#define RectangularTemplate_HH

/*
 **************************** includes **********************************
 */

#include <vector>

#include <cstdio>

#include <euclid/GridOffset.hh>
#include <euclid/GridPoint.hh>
#include <euclid/GridTemplate.hh>

/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class RectangularTemplate : public GridTemplate
{
 public:

  // Constructor

  RectangularTemplate(double length, double width);
  
  // Destructor

  ~RectangularTemplate(void);
  
  // Print the offset list to the given stream.  This is used for debugging.

  void printOffsetList(FILE *stream);
  
  // Access methods

  double getLength(void)
  {
    return _length;
  }

  double getWidth(void)
  {
    return _width;
  }
  
 private:

  // The box dimensions

  double _length;
  double _width;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("RectangularTemplate");
  }
  
};


#endif
