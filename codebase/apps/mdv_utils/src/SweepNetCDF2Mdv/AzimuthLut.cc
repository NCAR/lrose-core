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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:13 $
//   $Id: AzimuthLut.cc,v 1.2 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AzimuthLut: Class for handling the azimuth lookup table.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include "AzimuthLut.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

AzimuthLut::AzimuthLut(const double beamwidth) :
  _beamWidth(beamwidth)
{
  setBeamWidth(beamwidth);
}

  
/*********************************************************************
 * Destructor
 */

AzimuthLut::~AzimuthLut()
{
  // Do nothing
}


/*********************************************************************
 * clear() - Clear out the current lookup table values.  This method
 *           resets all of the azimuth indices to -1;
 */

void AzimuthLut::clear()
{
  vector< int >::iterator index;
  
  for (index = _azimuthIndices.begin(); index != _azimuthIndices.end();
       ++index)
    *index = -1;
}


/*********************************************************************
 * setBeamWidth() - Set the beam width to the given value.  This method
 *                  clears out all of the old index values and reinitializes
 *                  the lookup table at the new size.
 */

void AzimuthLut::setBeamWidth(const double beamwidth)
{
  // Clear out the old indicies

  _azimuthIndices.erase(_azimuthIndices.begin(), _azimuthIndices.end());
  
  // Initialize the lookup table

  int num_indices = (int)(360.0 / beamwidth);
  
  for (int i = 0; i < num_indices; ++i)
    _azimuthIndices.push_back(-1);
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
