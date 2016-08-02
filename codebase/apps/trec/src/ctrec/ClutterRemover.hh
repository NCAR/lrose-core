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
 *   $Id: ClutterRemover.hh,v 1.6 2016/03/06 23:28:57 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ClutterRemover.hh : header file for the ClutterRemover class.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ClutterRemover_HH
#define ClutterRemover_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>

#include <dataport/port_types.h>
#include <Mdv/MdvxField.hh>

#include "DataDetrender.hh"
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

class ClutterRemover
{
 public:

  // Constructor

  ClutterRemover(const double variance_thresh,
		 const int variance_radius,
		 const double min_data_value,
		 const double max_data_value,
		 DataDetrender *detrender,
		 const bool debug_flag = false);
  
  // Destructor

  ~ClutterRemover(void);
  
  // Remove the clutter from the given image.

  void run(fl32 *image, const int nx, const int ny,
	   const double bad_data_value);

  void run(MdvxField &field,
	   const double bad_data_value);
  
 private:

  bool _debugFlag;
  
  // Algorithm parameters

  double _minDataValue;
  double _maxDataValue;
  double _varianceThresh;
  int _varianceRadius;
  
  // Detrending algorithm

  DataDetrender *_detrender;
  
  // Variance grid used in calculations

  double *_variance;
  int _nx;
  int _ny;
  
  // Disallow the copy constructor and assignment operator

  ClutterRemover(const ClutterRemover&);
  const ClutterRemover& operator=(const ClutterRemover&);
  
};


#endif
