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
 * @file GridProj.hh 
 * @brief Projection info for a 2d grid.
 * @class GridProj
 * @brief Projection info for a 2d grid.
 *
 * Members public as its a 'struct-like' class
 */

#ifndef GRID_PROJ_H
#define GRID_PROJ_H
#include <Mdv/DsMdvx.hh>

//------------------------------------------------------------------
class GridProj 
{
public:

  /**
   * Constructor empty
   */
  GridProj(void);

  /**
   * Constructor
   * @param[in] hdr   MDV field header from which to get projection info.
   */
  GridProj(const Mdvx::field_header_t &hdr);

  /**
   * Destructor
   */
  virtual ~GridProj(void);

  /**
   * operator==
   * @param[in] g  Object to compare to
   */
  bool operator==(const GridProj &g) const;

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * @return true if its a full circle (only for the correct projection type,
   *  PROJ_POLAR_RADAR, PROJ_RADIAL)
   */
  bool isCircle(void) const;

  int _nx;     /**< Number of gridpoints x */
  int _ny;     /**< Number of gridpoints x */
  double _dx;  /**< gridpoint size x (from MDV) */
  double _dy;  /**< gridpoint size y (from MDV) */
  double _x0;  /**< lower left grid location (from MDV) */
  double _y0;  /**< lower left grid location (from MDV) */
  double _lat; /**< Latitude of x=0,y=0 */
  double _lon; /**< Longitude of x=0,y=0 */
  int _proj_type; /**< MDV projection type */

protected:
private:

};

#endif
