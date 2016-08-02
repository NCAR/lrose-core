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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/06 23:28:57 $
 *   $Id: DataDetrender.hh,v 1.3 2016/03/06 23:28:57 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DataDetrender.hh : header file for the DataDetrender class.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DataDetrender_HH
#define DataDetrender_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
using namespace std;

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

class DataDetrender
{
 public:

  // Constructor

  DataDetrender(const double thr_dbz,
		bool debug_flag = false);
  
  // Destructor

  ~DataDetrender(void);
  
  // Detrend the data in the given image.
  //
  // Returns true if the detrending was successful, false otherwise.

  bool run(const float *image,
	   const int nx, const int ny,
	   const double bad_data_value,
	   const int min_x, const int min_y,
	   const int max_x, const int max_y,
	   const int x_center, const int y_center);
  
  ////////////////////
  // Access methods //
  ////////////////////

  double getLsfValue(int x, int y)
  {
    if (x < 0 || x >= _nx ||
	y < 0 || y >= _ny)
      return _badDataValue;
    
    return _lsfGrid[x + (_nx * y)];
  }
  
 private:

  static const double EPSILON;
  
  bool _debugFlag;
  
  // Least squares data grid

  double *_lsfGrid;
  int _nx;
  int _ny;
  double _badDataValue;
  
  // Algorithm parameters

  double _thrDbz;
  
  // Disallow the copy constructor and assignment operator

  DataDetrender(const DataDetrender&);
  const DataDetrender& operator=(const DataDetrender&);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("DataDetrender");
  }
  
};


#endif
